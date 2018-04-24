#include "global.h"
#include "StepMania.h" //to assist with debugging
#include "RageLog.h"
#include "RageUtil.h"	// for ssprintf, arraylen

#include "io/P3IO.h"
#include "arch/USB/USBDriver_Impl.h"

static bool COM4SET=false;
static Preference<PSTRING> m_COM4PORT("P3IOCOM4PORT", ""); //COM2
serial::Serial P3IO::com4;
uint8_t P3IO::acio_request[256];
uint8_t P3IO::acio_response[256];

PSTRING P3IO::m_sInputError;
int P3IO::m_iInputErrorCount = 0;
static int connect_index = -1; //used for determining if we find the mouse
static uint8_t packet_sequence = 0;
static uint8_t packets_since_keepalive = 0;
// message timeout, in microseconds (so, 1 ms)
const int REQ_TIMEOUT = 1000;

uint8_t pchunk[3][4];
// p3io correlated pid/vid
//first set is what a chimera uses, second set is a real ddr io (from an hd black cab), the last set is actually a mouse I use for testing!

//has mouse debugger
//const uint16_t P3IO_VENDOR_ID[3] = { 0x0000, 0x1CCF, 0x046D};
//const uint16_t P3IO_PRODUCT_ID[3] = { 0x5731, 0x8008, 0xC077};


const uint16_t P3IO_VENDOR_ID[2] = { 0x0000, 0x1CCF };
const uint16_t P3IO_PRODUCT_ID[2] = { 0x5731, 0x8008 };
uint8_t p_light_payload[] = { 0, 0, 0, 0, 0 };
const unsigned NUM_P3IO_CHECKS_IDS = ARRAYLEN(P3IO_PRODUCT_ID);
int interrupt_ep = 0x83;
int bulk_write_to_ep = 0x02;
int bulk_read_from_ep = 0x81;
bool m_bConnected = false;

uint8_t P3IO::p3io_request[256];
uint8_t P3IO::p3io_response[256];
char debug_message[2048];
int bulk_reply_size=0;
bool baud_pass=false;
bool hdxb_ready=false;



bool P3IO::DeviceMatches(int iVID, int iPID)
{

	for (unsigned i = 0; i < NUM_P3IO_CHECKS_IDS; ++i)
		if (iVID == P3IO_VENDOR_ID[i] && iPID == P3IO_PRODUCT_ID[i])
			return true;	

	return false;
}

bool P3IO::Open()
{

	if (m_COM4PORT.Get().length()>1)
	{
		COM4SET=true;
		com4.setPort(m_COM4PORT.Get().c_str());
	}

	packet_sequence = 0;
	/* we don't really care which PID works, just if it does */
	for (unsigned i = 0; i < NUM_P3IO_CHECKS_IDS; ++i)
		if (OpenInternal(P3IO_VENDOR_ID[i], P3IO_PRODUCT_ID[i]))
		{
			LOG->Info("P3IO Driver:: Connected to index %d", i);
			connect_index = i;
			if (i == 2)
			{
				interrupt_ep = 0x81; //debug mouse
				LOG->Info("init p3io watch dog ");
				InitHDAndWatchDog();
			}
			else
			{
				LOG->Info("init p3io watch dog ");
				InitHDAndWatchDog();
			}
			m_bConnected = true;
			openHDXB();
			usleep(906250); //capture waits this long
			sendUnknownCommand();
			FlushBulkReadBuffer();


			//to assist with debugging
			//ExitGame();


			baud_pass=spamBaudCheck();
			
			// say the baud check passed anyway
			baud_pass=true;

			
			if(baud_pass)
			{
				nodeCount();
				getVersion(); 
				initHDXB2(); 
				initHDXB3(); 
				initHDXB4(); 
				pingHDXB();
				HDXBAllOnTest();
				hdxb_ready=true;
			}
			
			return m_bConnected;
		}
	m_bConnected = false;
	return m_bConnected;
}

void P3IO::HDXBAllOnTest()
{
	uint8_t all_on[0x0d]={
	0x00,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x7f,
	0x7f,
	0x7f,
	0x7f,
	0x7f,
	0x7f
	};
	LOG->Info("**************HDXB ALL ON TEST:");
	writeHDXB(all_on, 0xd, HDXB_SET_LIGHTS);
}



//ok seriously what was I drinking / smoking here
//i dont even know how or why this works but it does
//basically we need to get all 3 4-byte chunks read in for this to be COMPLETE
//but the maximum payload size is 16 bytes?  so I guess theres a circumstance when it could poop out 16?
//using libusb1.x I could only get 4x chunks, and 0.x seems to give 12 byte chunks.. I just dont even know anymore
//someone smarter than me make this better please
bool P3IO::interruptRead(uint8_t* dataToQueue)
{
	m_iInputErrorCount = 0;
	int iExpected = 16; //end point size says max packet of 16, but with libusb1 I only ever got 12 in chunks of 4 every consequtive read
	uint8_t chunk[4][4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t tchunk[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool interruptChunkFlags[3] = { false, false, false }; //i guess only 12 is needed? lets work with that
	bool allChunks = false;
	int currChunk = 0;
	while (!allChunks)
	{
		int iResult = m_pDriver->InterruptRead(interrupt_ep, (char*)tchunk, iExpected, REQ_TIMEOUT);

		//LOG->Warn( "p3io read, returned %i: %s\n", iResult, m_pDriver->GetError() );
		if (iResult>0)
		{
			m_iInputErrorCount = 0;
			//LOG->Info("P3IO Got %d bytes of data: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",iResult, tchunk[0],tchunk[1],tchunk[2],tchunk[3],tchunk[4],tchunk[5],tchunk[6],tchunk[7],tchunk[8],tchunk[9],tchunk[10],tchunk[11]);
			if (iResult >= 12 || connect_index == 2)
			{
				//we have EVERYTHING
				interruptChunkFlags[0] = true;
				interruptChunkFlags[1] = true;
				interruptChunkFlags[2] = true;
				for (int i = 0; i<iResult; i++)
				{
					chunk[i / 4][i % 4] = tchunk[i]; //array data is continuous
				}
			}
			else
			{
				if (iResult == 4)
				{
					//if we get the beginning, reset ourselves
					if (tchunk[0] == 0x80)
					{
						interruptChunkFlags[0] = false;
						interruptChunkFlags[1] = false;
						interruptChunkFlags[2] = false;
						currChunk = 0;
					}
					for (int i = 0; i<4; i++)
					{
						chunk[currChunk][i] = tchunk[i];
					}
					interruptChunkFlags[currChunk] = true;
					currChunk++;
				}
				else
				{
					//LOG->Info("P3IO driver got weird number of bytes (%d) on an interrupt read!",iResult);
				}
			}


			//check all chunks are accounted for
			if (interruptChunkFlags[0] == true)
			{
				if (interruptChunkFlags[1] == true)
				{
					if (interruptChunkFlags[2] == true)
					{
						allChunks = true;
					}
				}
			}


		}
		else
		{
			//we got nothing, increment error and then spit out last state
			m_iInputErrorCount++;
		}
		if (m_iInputErrorCount>2)
		{
			break;
		}
	}

	//if we didnt get what we need, return the previous data
	if (allChunks == false)
	{
		for (int i = 0; i<3; i++)
		{
			for (int j = 0; j<4; j++)
			{
				chunk[i][j] = pchunk[i][j];
			}
		}
	}
	else //copy our current state to the previous state
	{
		for (int i = 0; i<3; i++)
		{
			for (int j = 0; j<4; j++)
			{
				pchunk[i][j] = chunk[i][j];
			}
		}
	}

	//format the data in the way we expect -- harkens back to the UDP p3io streamer
	uint8_t DDR_P1_PAD_UP = checkInput(pchunk[0][1], DDR_PAD_UP);
	uint8_t DDR_P1_PAD_DOWN = checkInput(pchunk[0][1], DDR_PAD_DOWN);
	uint8_t DDR_P1_PAD_LEFT = checkInput(pchunk[0][1], DDR_PAD_LEFT);
	uint8_t DDR_P1_PAD_RIGHT = checkInput(pchunk[0][1], DDR_PAD_RIGHT);

	uint8_t DDR_P1_CP_UP = checkInput(pchunk[0][3], DDR_CP_UP_P1);
	uint8_t DDR_P1_CP_DOWN = checkInput(pchunk[0][3], DDR_CP_DOWN_P1);
	uint8_t DDR_P1_CP_LEFT = checkInput(pchunk[0][1], DDR_CP_LEFT);
	uint8_t DDR_P1_CP_RIGHT = checkInput(pchunk[0][1], DDR_CP_RIGHT);
	uint8_t DDR_P1_CP_SELECT = checkInput(pchunk[0][1], DDR_CP_SELECT);

	uint8_t DDR_P2_PAD_UP = checkInput(pchunk[0][2], DDR_PAD_UP);
	uint8_t DDR_P2_PAD_DOWN = checkInput(pchunk[0][2], DDR_PAD_DOWN);
	uint8_t DDR_P2_PAD_LEFT = checkInput(pchunk[0][2], DDR_PAD_LEFT);
	uint8_t DDR_P2_PAD_RIGHT = checkInput(pchunk[0][2], DDR_PAD_RIGHT);

	uint8_t DDR_P2_CP_UP = checkInput(pchunk[0][3], DDR_CP_UP_P2);
	uint8_t DDR_P2_CP_DOWN = checkInput(pchunk[0][3], DDR_CP_DOWN_P2);
	uint8_t DDR_P2_CP_LEFT = checkInput(pchunk[0][2], DDR_CP_LEFT);
	uint8_t DDR_P2_CP_RIGHT = checkInput(pchunk[0][2], DDR_CP_RIGHT);
	uint8_t DDR_P2_CP_SELECT = checkInput(pchunk[0][2], DDR_CP_SELECT);

	uint8_t DDR_OP_TEST = checkInput(pchunk[0][3], DDR_TEST);
	uint8_t DDR_OP_SERVICE = checkInput(pchunk[0][3], DDR_SERVICE);
	uint8_t DDR_OP_COIN1 = checkInput(pchunk[0][3], DDR_COIN);
	//uint8_t DDR_OP_COIN2=checkInput(pchunk[0][3],DDR_COIN);

	//init the memory... yeah I'm lazy
	dataToQueue[0] = 0;
	dataToQueue[1] = 0;
	dataToQueue[2] = 0;
	if (DDR_P1_PAD_UP) dataToQueue[0] |= BYTE_BIT_M_TO_L(0);
	if (DDR_P1_PAD_DOWN) dataToQueue[0] |= BYTE_BIT_M_TO_L(1);
	if (DDR_P1_PAD_LEFT) dataToQueue[0] |= BYTE_BIT_M_TO_L(2);
	if (DDR_P1_PAD_RIGHT) dataToQueue[0] |= BYTE_BIT_M_TO_L(3);
	if (DDR_P2_PAD_UP) dataToQueue[0] |= BYTE_BIT_M_TO_L(4);
	if (DDR_P2_PAD_DOWN) dataToQueue[0] |= BYTE_BIT_M_TO_L(5);
	if (DDR_P2_PAD_LEFT) dataToQueue[0] |= BYTE_BIT_M_TO_L(6);
	if (DDR_P2_PAD_RIGHT) dataToQueue[0] |= BYTE_BIT_M_TO_L(7);

	if (DDR_P1_CP_UP) dataToQueue[1] |= BYTE_BIT_M_TO_L(0); //up and down buttons constantly pressed when reading with libusbk....
	if (DDR_P1_CP_DOWN) dataToQueue[1] |= BYTE_BIT_M_TO_L(1); //need to get captures of these inputs with oitg to figure out whats up
	if (DDR_P1_CP_LEFT) dataToQueue[1] |= BYTE_BIT_M_TO_L(2);
	if (DDR_P1_CP_RIGHT) dataToQueue[1] |= BYTE_BIT_M_TO_L(3);
	if (DDR_P2_CP_UP) dataToQueue[1] |= BYTE_BIT_M_TO_L(4);//so for now I commented them out
	if (DDR_P2_CP_DOWN) dataToQueue[1] |= BYTE_BIT_M_TO_L(5);//so the game is playable
	if (DDR_P2_CP_LEFT) dataToQueue[1] |= BYTE_BIT_M_TO_L(6);
	if (DDR_P2_CP_RIGHT) dataToQueue[1] |= BYTE_BIT_M_TO_L(7);

	if (DDR_OP_SERVICE) dataToQueue[2] |= BYTE_BIT_M_TO_L(0);
	if (DDR_OP_TEST) dataToQueue[2] |= BYTE_BIT_M_TO_L(1);
	if (DDR_OP_COIN1) dataToQueue[2] |= BYTE_BIT_M_TO_L(2);
	//if (DDR_OP_COIN2) dataToQueue[2] |= BYTE_BIT_M_TO_L(3); //my second coin switch is broken, I am just assuming this is the right constant...
	if (DDR_P1_CP_SELECT) dataToQueue[2] |= BYTE_BIT_M_TO_L(4);
	if (DDR_P2_CP_SELECT) dataToQueue[2] |= BYTE_BIT_M_TO_L(5);

	if (!allChunks){
		Close();
	}
	return allChunks;

}

bool P3IO::sendUnknownCommand()
{
	if (COM4SET) return true;
	uint8_t unknown[] = {
	0xaa,
	0x04,
	0x0a,
	0x32,
	0x01,
	0x00
	};
	//LOG->Info("**************HDXB UNKNOWN COMMAND:");
	return WriteToBulkWithExpectedReply( unknown, false, true);
}


//sent every 78125us...
bool P3IO::pingHDXB()
{
if (COM4SET)
	{
		//LOG->Info("**************HDXB Serial PING:");
		uint8_t hdxb_ping_command[] = {
			0xaa,//ACIO_PACKET_START
			0x03,//HDXB
			0x01,//type?
			0x10,//opcode
			0x00,
			0x00,
			0x14//CHECKSUM
		};
		com4.write(hdxb_ping_command,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
	}
	else
	{
		uint8_t hdxb_ping_command[] = {
			0xaa,
			0x0b,
			0x02,
			0x3a,
			0x00,
			0x07,
			0xaa,//ACIO_PACKET_START
			0x03,//HDXB
			0x01,//type?
			0x10,//opcode
			0x00,
			0x00,
			0x14//CHECKSUM
		};
		//LOG->Info("**************HDXB P3IO PING:");
		WriteToBulkWithExpectedReply(hdxb_ping_command, false, true);
		return readHDXB(0x7e); // now flush it according to captures
	}
	return true;

}

bool P3IO::spamBaudCheck()
{
	bool found_baud=false;
	if (COM4SET)
	{
		//LOG->Info("**************HDXB Serial BAUD Routine:");
		found_baud= ACIO::baudCheck(com4);
	}
	else
	{
		uint8_t com_baud_command[] = {
		0xaa, // packet start
		0x0f, // length
		0x05, // sequence
		0x3a, // com write
		0x00, // virtual com port
		0x0b, // num bytes
		0xaa, // baud check, escaped 0xaa
		0xaa,	0xaa,	0xaa,	0xaa,	0xaa,	0xaa,	0xaa,	0xaa,	0xaa,	0xaa	};

		for (int i=0;i<50;i++)
		{
			//LOG->Info("**************HDXB P3IO BAUD:");
			WriteToBulkWithExpectedReply(com_baud_command, false, true);
			readHDXB(0x40);
			if (bulk_reply_size>=16 && p3io_response[1]==0x0F && p3io_response[4]==0x0B &&  p3io_response[5]==0xAA && p3io_response[6]==0xAA && p3io_response[7]==0xAA && p3io_response[8]==0xAA && p3io_response[9]==0xAA && p3io_response[10]==0xAA && p3io_response[11]==0xAA && p3io_response[12]==0xAA && p3io_response[13]==0xAA && p3io_response[14]==0xAA && p3io_response[15]==0xAA)
			{
			
				//LOG->Info("**************HDXB P3IO GOT BAUD RATE! Flushing...");
				readHDXB(0x40); // now flush it
			
				readHDXB(0x40); // now flush it
				found_baud=true;
				break;
			}
		}
	}

	return found_baud;
	

}

bool P3IO::nodeCount()
{
	if (COM4SET)
	{
		uint8_t nodes[]={0xaa,0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02}; 
		com4.write(nodes,8);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);//flush
		return true;
	}
	else
	{
		// 00    00    01    00    PP    07    CC -- 01 is the command for enumeration, PP is 01 (payload length), 07 is the payload with a reply of 7 in it, CC is checksum
		uint8_t nodes[]={0xaa,0x0e,0x01,0x3a,0x00,0x08,0xaa,0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02}; 
		//LOG->Info("**************HDXB P3IO Get Node Count:");
		WriteToBulkWithExpectedReply(nodes, false, true);
		readHDXB(0x40);
		return readHDXB(0x40); // now flush it
	}
}

bool P3IO::getVersion()
{
	if(COM4SET)
	{
		uint8_t ICCB_1[]={
			0xaa,//ACIO START
			0x01,//ICCB1
			0x00,//TYPE?
			0x02,//OPCODE
			0x00,
			0x00,
			0x03,
		};
		uint8_t ICCB_2[]={
			0xaa,//ACIO START
			0x02,//ICCB2
			0x00,//TYPE?
			0x02,//OPCODE
			0x00,
			0x00,
			0x04,
		};
		uint8_t HDXB[]={
			0xaa,//ACIO START
			0x03,//HDXB
			0x00,//TYPE?
			0x02,//OPCODE
			0x00,
			0x00,
			0x05,
		};
		//LOG->Info("**************ICCB1 Serial Get Version:");

		com4.write(ICCB_1,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		//LOG->Info("**************ICCB2 Serial Get Version:");
		com4.write(ICCB_2,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		//LOG->Info("**************HDXB Serial Get Version:");
		com4.write(HDXB,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		return true;
	}
	else
	{
		//should build this but.. meh... lazy, manually define it
		uint8_t ICCB_1[]={
			0xaa,
			0x0d,
			0x01,
			0x3a,
			0x00,
			0x07,
			0xaa,//ACIO START
			0x01,//ICCB1
			0x00,//TYPE?
			0x02,//OPCODE
			0x00,
			0x00,
			0x03,
		};
		uint8_t ICCB_2[]={
			0xaa,
			0x0d,
			0x01,
			0x3a,
			0x00,
			0x07,
			0xaa,//ACIO START
			0x02,//ICCB2
			0x00,//TYPE?
			0x02,//OPCODE
			0x00,
			0x00,
			0x04,
		};
		uint8_t HDXB[]={
			0xaa,
			0x0d,
			0x01,
			0x3a,
			0x00,
			0x07,
			0xaa,//ACIO START
			0x03,//HDXB
			0x00,//TYPE?
			0x02,//OPCODE
			0x00,
			0x00,
			0x05,
		};
		//LOG->Info("**************ICCB1 P3IO Get Version:");
		WriteToBulkWithExpectedReply(ICCB_1, false, true);
		readHDXB(0x7e);
		readHDXB(0x7e);
		//LOG->Info("**************ICCB2 P3IO Get Version:");
		WriteToBulkWithExpectedReply(ICCB_2, false, true);
		readHDXB(0x7e);
		readHDXB(0x7e);
		//LOG->Info("**************HDXB P3IO Get Version:");
		WriteToBulkWithExpectedReply(HDXB, false, true);
		readHDXB(0x7e);
		return readHDXB(0x7e);
	}
}

bool P3IO::initHDXB2()
{
	if(COM4SET)
	{
		//should build this but.. meh... lazy, manually define it
		//init into opcode 3?
		uint8_t hdxb_op3[]={
			0xaa,//ACIO START
			0x03,//HDXB
			0x00,//TYPE?
			0x03,//OPCODE
			0x00,
			0x00,
			0x06,
		};

		uint8_t ICCB1_op3[]={0xaa,0x01,0x00,0x03,0x00,0x00,0x04};
		uint8_t ICCB2_op3[]={0xaa,0x02,0x00,0x03,0x00,0x00,0x05};
	
		//LOG->Info("**************ICCB1 Serial INIT mode 3:");
		com4.write(ICCB1_op3,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		//LOG->Info("**************ICCB2 Serial INIT mode 3:");
		com4.write(ICCB2_op3,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		//LOG->Info("**************HDXB Serial INIT mode 3:");
		com4.write(hdxb_op3,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);// now flush it according to captures
		return true; 
	}
	else
	{
		//should build this but.. meh... lazy, manually define it
		//init into opcode 3?
		uint8_t hdxb_op3[]={
			0xaa,
			0x0d,
			0x01,
			0x3a,
			0x00,
			0x07,
			0xaa,//ACIO START
			0x03,//HDXB
			0x00,//TYPE?
			0x03,//OPCODE
			0x00,
			0x00,
			0x06,
		};

		uint8_t ICCB1_op3[]={0xaa,0x0d,0x02,0x3a,0x00,0x07,0xaa,0x01,0x00,0x03,0x00,0x00,0x04};
		uint8_t ICCB2_op3[]={0xaa,0x0d,0x03,0x3a,0x00,0x07,0xaa,0x02,0x00,0x03,0x00,0x00,0x05};
	
		//LOG->Info("**************ICCB1 P3IO INIT mode 3:");
		WriteToBulkWithExpectedReply(ICCB1_op3, false, true);
		readHDXB(0x7e);
		//LOG->Info("**************ICCB2 P3IO INIT mode 3:");
		WriteToBulkWithExpectedReply(ICCB2_op3, false, true);
		readHDXB(0x7e);
		//LOG->Info("**************HDXB P3IO INIT mode 3:");
		WriteToBulkWithExpectedReply(hdxb_op3, false, true);
		readHDXB(0x7e);
		return readHDXB(0x7e); // now flush it according to captures
	}
}


bool P3IO::initHDXB3()
{
	if(COM4SET)
	{
		//should build this but.. meh... lazy, manually define it
		uint8_t hdxb_type[]={
			0xaa,//ACIO START
			0x03,//HDXB
			0x01,//TYPE?
			0x00,//OPCODE
			0x00,
			0x00,
			0x04,
		};
		uint8_t iccb1_type[]={0xaa,0x01,0x01,0x00,0x00,0x00,0x02};
		uint8_t iccb2_type[]={0xaa,0x02,0x01,0x00,0x00,0x00,0x03};
	
		//LOG->Info("**************ICCB1 Serial INIT type 0:");
		com4.write(iccb1_type,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		//LOG->Info("**************ICCB2 Serial  INIT type 0:");
		com4.write(iccb2_type,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		//LOG->Info("**************HDXB Serial  INIT type 0:");
		com4.write(hdxb_type,7);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		return true;
	}
	else
	{

		//should build this but.. meh... lazy, manually define it
		uint8_t hdxb_type[]={
			0xaa,
			0x0d,
			0x01,
			0x3a,
			0x00,
			0x07,
			0xaa,//ACIO START
			0x03,//HDXB
			0x01,//TYPE?
			0x00,//OPCODE
			0x00,
			0x00,
			0x04,
		};
		uint8_t iccb1_type[]={0xaa,0x0b,0x0a,0x3a,0x00,0x07,0xaa,0x01,0x01,0x00,0x00,0x00,0x02};
		uint8_t iccb2_type[]={0xaa,0x0b,0x0e,0x3a,0x00,0x07,0xaa,0x02,0x01,0x00,0x00,0x00,0x03};
	
		//LOG->Info("**************ICCB1 P3IO INIT type 0:");
		WriteToBulkWithExpectedReply(iccb1_type, false, true);
		readHDXB(0x7e);
		readHDXB(0x7e); // now flush it according to captures
		//LOG->Info("**************ICCB2 P3IO INIT type 0:");
		WriteToBulkWithExpectedReply(iccb2_type, false, true);
		readHDXB(0x7e);
		readHDXB(0x7e); // now flush it according to captures
		//LOG->Info("**************HDXB P3IO INIT type 0:");
		WriteToBulkWithExpectedReply(hdxb_type, false, true);
		readHDXB(0x7e);
		return readHDXB(0x7e); // now flush it according to captures
	}
}


bool P3IO::initHDXB4()
{
	if(COM4SET)
	{
		//should build this but.. meh... lazy, manually define it
		uint8_t breath_of_life[]={
			0xaa,//ACIO START
			0x03,//HDXB
			0x01,//TYPE?
			0x28,//OPCODE
			0x00,
			0x02,
			0x00,
			0x00,
			0x2e
		};
		//LOG->Info("**************HDXB Serial INIT4:");
		com4.write(breath_of_life,9);
		usleep(72000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
		return true;
	}
	else
	{
		//should build this but.. meh... lazy, manually define it
		uint8_t breath_of_life[]={
			0xaa,
			0x0d,
			0x01,
			0x3a,
			0x00,
			0x09,
			0xaa,//ACIO START
			0x03,//HDXB
			0x01,//TYPE?
			0x28,//OPCODE
			0x00,
			0x02,
			0x00,
			0x00,
			0x2e
		};
		//LOG->Info("**************HDXB P3IO INIT4:");
		return WriteToBulkWithExpectedReply(breath_of_life, false, true);
		//We are NOT reading because the capture says we don't at this point!
	}
}

bool P3IO::openHDXB()
{

	if (COM4SET)
	{
		com4.open();
		com4.setBaudrate(38400);
		//LOG->Info("**************HDXB SERIAL OPEN:");
		return true;
	}
	else
	{

	
		uint8_t com_open_command[] = {
		0xaa, // packet start
		0x05, // length
		0x06, // sequence
		0x38, // com open opcode
		0x00, // virtual com port
		0x00, // @ baud
		0x03  // choose speed: ?, ?, 19200, 38400, 57600
		};
		//LOG->Info("**************HDXB P3IO OPEN:");
		return WriteToBulkWithExpectedReply(com_open_command, false, true);
	}
}

bool P3IO::readHDXB(int len)
{
	usleep(36000); // 36 ms wait
	uint8_t com_read_command[] = {
		0xaa, // packet start
		0x04, // length
		0x0f, // sequence
		0x3b, // com read opcode
		0x00, // virtual com port
		0x7e  // 7e is 126 bytes to read
	};
	com_read_command[5]=len&0xFF; // plug in passed in len
	//LOG->Info("**************HDXB P3IO Read:");
	return WriteToBulkWithExpectedReply(com_read_command, false, true);
}

//pass in a raw payload only
bool P3IO::writeHDXB(uint8_t* payload, int len, uint8_t opcode)
{
	if(COM4SET)
	{
	
		//frame: 03:01:12:00:0d
		//payload consists of 00:ff:ff:ff:ff:ff:ff:7f:7f:7f:7f:7f:7f
		//must also accomodate acio packet of: aa:03:01:28:00:02:00:00:2e
		acio_request[0]=0x03; // hdxb device id
		acio_request[1]=0x01; // type?
		acio_request[2]=opcode; // set lights?
		acio_request[3]=0x00; //
		acio_request[4]=len & 0xFF; // length
	
		//len should be 0x0d
		memcpy( &acio_request[5],  payload, len * sizeof( uint8_t ) );

		//03:01:12:00:0d:00:ff:ff:ff:ff:ff:ff:7f:7f:7f:7f:7f:7f
		len++; // add checksum to length of payload
		//len is now 0x0e

		//uint8_t unescaped_acio_length = len & 0xFF; // but we dont need this, we need number of bytes to write to acio bus

		//get escaped packet and length
		len+=5; //acio frame bytes added
		len = ACIO::prep_acio_packet_for_transmission(acio_request,len); 
		com4.write(acio_request,len);
		usleep(36000); // 36 ms wait
		ACIO::read_acio_packet(com4,acio_response);
	}
	else
	{
		//frame: 03:01:12:00:0d
		//payload consists of 00:ff:ff:ff:ff:ff:ff:7f:7f:7f:7f:7f:7f
		//must also accomodate acio packet of: aa:03:01:28:00:02:00:00:2e
		p3io_request[0]=0x03; // hdxb device id
		p3io_request[1]=0x01; // type?
		p3io_request[2]=opcode; // set lights?
		p3io_request[3]=0x00; //
		p3io_request[4]=len & 0xFF; // length
	
		//len should be 0x0d
		memcpy( &p3io_request[5],  payload, len * sizeof( uint8_t ) );

		//03:01:12:00:0d:00:ff:ff:ff:ff:ff:ff:7f:7f:7f:7f:7f:7f
		len++; // add checksum to length of payload
		//len is now 0x0e

		//uint8_t unescaped_acio_length = len & 0xFF; // but we dont need this, we need number of bytes to write to acio bus

		//get escaped packet and length
		len+=5; //acio frame bytes added
		len = ACIO::prep_acio_packet_for_transmission(p3io_request,len); 

		//we are now ready to transmit over p3io channels...
		uint8_t p3io_acio_message[256];
		p3io_acio_message[0]=0xaa;
		p3io_acio_message[1]=(len+4) & 0xFF;
		p3io_acio_message[2]=0;
		p3io_acio_message[3]=0x3a; //com port write
		p3io_acio_message[4]=0; // actual virtual com port number to use
		p3io_acio_message[5]=len&0xFF; //num bytes to write on the acio bus
		memcpy( &p3io_acio_message[6],  p3io_request, len * sizeof( uint8_t ) ); // stuff in a p3io wrapper
		//LOG->Info("**************HDXB P3IO LIGHT Write:");
		WriteToBulkWithExpectedReply(p3io_acio_message, false, true);
		//LOG->Info("**************HDXB P3IO Get Ping:");
	
	}
	return pingHDXB();
}

bool P3IO::writeLights(uint8_t* payload)
{
	packets_since_keepalive++;

	//if not connected do nothing
	if (!m_bConnected)
	{
		//LOG->Info("P3IO io driver is in disconnected state! Not doing anything in write lights!");
		return false;
	}

	//arbitrary keep alive signal. maybe can dial this back? send one evry 4 attempts
	//lights are CONSTANTLY sending in this architecture
	packets_since_keepalive %= 6;
	

	
	//compare last payload with this payload -- prevent useless chatter on USB so input has best poll rate
	if ( (memcmp(p_light_payload, payload, sizeof(p_light_payload))!=0) || (packets_since_keepalive == 0) )
	{	//if they dont match...
		
		//copy new payload to previous payload so I can check it next round
		memcpy(p_light_payload, payload, sizeof(p_light_payload));
	
		//make a light message
		uint8_t light_message[] = { 0xaa, 0x07, 0x00, 0x24, ~payload[4], payload[3], payload[2], payload[1], payload[0] };
		//LOG->Info("P3io driver Sending light message: %02X %02X %02X %02X %02X %02X %02X %02X %02X", light_message[0],light_message[1],light_message[2],light_message[3],light_message[4],light_message[5],light_message[6],light_message[7],light_message[8]);

		//send message
		return WriteToBulkWithExpectedReply(light_message, false);
	}
	return true;
}



bool P3IO::WriteToBulkWithExpectedReply(uint8_t* message, bool init_packet, bool output_to_log)
{
	//LOG->Info("p3io driver bulk write: Applying sequence");
	//first figure out what our message parameters should be based on what I see here and our current sequence
	packet_sequence++;
	packet_sequence %= 16;
	if (!init_packet)
	{
		message[2] = packet_sequence;
	}
	else
	{
		packet_sequence = message[2];
	}

	//max packet size is 64 bytes * 2 for escaped packets
	uint8_t response[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	int message_length = 5;
	int expected_response_size = 3; // handles packet start, length, sequence.. everything should be long than this though. Updates in the function

	message_length = message[1]+2;



	//**********THIS IS ALL WRONG WRONG WRONG WRONG WRONG... Well, the opcodes are right but the logic behind this is highly outdated... just only use this as a function lexicon
	//LOG->Info("Determining message type...");
	//LOG->Info("(%02X)",message[3]);

	//may not need this except as a lexicon -- com read throws it out the window
	/*
	switch (message[3])
	{


		case 0x01: // version query
			expected_response_size = 11;
			break;

		case 0x31: //coin query
			expected_response_size = 9;
			break;
	
		case 0x2f:// some query
			expected_response_size = 5;
			break;

		case 0x27: //cabinet type query
			expected_response_size = 5;
			break;


		case 0x24: //set light state?
			expected_response_size = 6;
			break;

		case 0x25: //get dongle
			expected_response_size = 45;
			break;
		case 0x3a: // com write
			expected_response_size = 5; /// basically an ack
			break;
	
		default:
			expected_response_size = 5;
	}
	*/
	//End horribly wrong segment
	
	
	//LOG->Info("Expected response size is %d", expected_response_size);
	//actually send our message with the parameters we determined
	//but first we need to escape our shit


	//LOG->Info("Escaping our shit");
	int bytes_to_write=message_length;
	uint8_t message2[258];
	message2[0]=message[0];
	int old_stream_position=1;
	for (int new_stream_position=1;new_stream_position<bytes_to_write;new_stream_position++)
	{
		if (message[old_stream_position]==0xAA || message[old_stream_position]==0xFF)
		{
			message2[new_stream_position]=0xFF;
			new_stream_position++;
			bytes_to_write++;
			message2[new_stream_position]=~message[old_stream_position];
		}
		else
		{
			message2[new_stream_position]=message[old_stream_position];
		}
		old_stream_position++;
	}
	//END DATA EScAPING CODE


	//prepare debug line
	for (int i=0;i<bytes_to_write;i++)
	{
		sprintf (debug_message+(i*3), "%02X ", message2[i]);
	}
	//if(output_to_log) LOG->Info("Send %d bytes - %s", bytes_to_write, debug_message);

	int iResult = m_pDriver->BulkWrite(bulk_write_to_ep, (char*)message2, bytes_to_write, REQ_TIMEOUT);

	if (iResult != bytes_to_write)
	{
		LOG->Info("P3IO message to send was truncated. Sent %d/%d bytes", iResult,bytes_to_write);
	}

	//get ready for the response
	//LOG->Info("Asking for reply...");
	iResult = GetResponseFromBulk(response, expected_response_size, output_to_log); //do a potentially fragmented read
	bulk_reply_size=iResult;


	//prepare debug line
	debug_message[0]=0;
	for (int i=0;i<bulk_reply_size;i++)
	{
		sprintf (debug_message+(i*3), "%02X ", response[i]);
	}
	//if(output_to_log) LOG->Info("Full response is %d/%d bytes - %s", bulk_reply_size,response[1]+2, debug_message);
	

	//if the unescaped data is too small according to the packet length byte...
	if (bulk_reply_size<(response[1]+2))
	{
		if(output_to_log) LOG->Info("Something is fishy, asking again...");
		bool overrider=false; //probably make this override more rubust but it is a giant hack anyway... hopefully not needed anymore
		if (iResult>1 && response[1]!=0xaa) overrider=true; // if we have something that looks like the start of the packet start vaguely
		//tell it to override looking for packet start, trimming off the leading 0xAAs and length.
		iResult += GetResponseFromBulk(response+iResult, response[1]-bulk_reply_size, output_to_log,overrider); //do a potentially fragmented read
		bulk_reply_size=iResult;

		//prepare debug line
		debug_message[0]=0;
		for (int i=0;i<bulk_reply_size;i++)
		{
			sprintf (debug_message+(i*3), "%02X ", response[i]);
		}
		if(output_to_log) LOG->Info("2nd attempt full response is %d/%d bytes - %s", bulk_reply_size,response[1]+2, debug_message);
	}
	for (int i=0;i<bulk_reply_size;i++)
	{
		p3io_response[i]=response[i];
	}
	return true;
}

//sometimes responses are split across 2 reads... could there be a THIRD read?
//this is a giant hack because data can be escaped which really fucks with length
int P3IO::GetResponseFromBulk(uint8_t* response, int expected_response_length, bool output_to_log, bool force_override)
{
	int totalBytesRead=0;
	int num_reads=0;
	int num_failed_reads=0;
	int escaped_bytes=0;
	bool flag_has_start_of_packet=false;
	bool flag_has_response_length=false;
	if (force_override)
	{
			flag_has_start_of_packet=true;
			flag_has_response_length=true;
	}
	//int responseLength = m_pDriver->BulkRead(bulk_read_from_ep, (char*)response, 128, REQ_TIMEOUT);
	//LOG->Info("begin while loop");
	while (totalBytesRead < (expected_response_length+2)) //sometimes a response is fragmented over multiple requests, try a another time
	{
		//if(output_to_log) LOG->Info("GETTING BULK OF EXPECTED SIZE: %02X/%02x",totalBytesRead,expected_response_length);
		int i=0;
		num_reads++;

		uint8_t response2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		int bytesReadThisRound = m_pDriver->BulkRead(bulk_read_from_ep, (char*)response2, 64, REQ_TIMEOUT);
		int escapedBytesThisRound=0;
		if (bytesReadThisRound <1) num_failed_reads++; //note that we got nothing from the read

		//DEBUG
		debug_message[0]=0;
		for (i=0;i<bytesReadThisRound;i++)
		{
			sprintf (debug_message+(i*3), "%02X ", response2[i]);
		}
		//if(output_to_log) LOG->Info("Bulk read round %d is %d bytes - %s", num_reads, bytesReadThisRound, debug_message);
		//END DEBUG


		if (bytesReadThisRound<1 && num_failed_reads>1)
		{
			LOG->Info("P3IO must have given up sending data we need (%d/%d bytes)",totalBytesRead,expected_response_length);
			break;
		}

		//unescape the bytes
		i=0;


		//Try and find start of packet in the data we read
		if (!flag_has_start_of_packet)
		{
			//LOG->Info("P3io not escaping sop");
			while(response2[0]!=0xAA && bytesReadThisRound>0)
			{
				//LOG->Info("Response buffer does NOT start with 0xAA, it starts with %02X. Shifting buffer left by one.",response2[0]);
				//shift everything over until we do
				for (i=0;i<bytesReadThisRound-1;i++)
				{
					response2[i]=response2[i+1];
					response2[i+1]=0;
				}
				bytesReadThisRound--;
			}
			if (response2[0]==0xaa) flag_has_start_of_packet=true;

		}
		
		//if we dont have start of packet, lets try to read again
		if (!flag_has_start_of_packet)
		{
			usleep(3000); //sleep for 3ms
			continue;
		}

		//at this point we are guarunteed to have start of packet which means the first byte is 0xAA
		//So now the next non-0xAA byte is the length of the packet

		//EDGE CASE! We read a 0xAA in a previous round (reflected in total bytes read)
		i=0; //assume start of packet is in main read buffer
		if (totalBytesRead<1) i=1; // if we have nothing in the main buffer and the flag_has_start_of_packet is set, it means the start of packet is in the current read buffer

		for (i;i<bytesReadThisRound;i++) //lets look at EVERYTHING we read
		{
			//response length is not set...
			if (!flag_has_response_length)
			{
				//if we see ANY character that is not AA... we have a response length at some point
				if (response2[i]!=0xAA)
				{
					//if(output_to_log) LOG->Info("Expected response length found! Setting new length to %02X",response2[i]);
					flag_has_response_length=true;
					expected_response_length=response2[i];
					i--;
					continue;
				}

				//if we haven't seen any other data and AA is present in slot 2
				if (response2[i]==0xAA)
				{
					bytesReadThisRound--;
					//shift all bytes in the array after this char to the left
					for (int j=i;j<bytesReadThisRound;j++)
					{
						response2[j]=response2[j+1];
					}
					i--;
					continue;
				}
			}
			else //we have the first 2 bytes of a packet such that we know the response length
			{
				//if we hit an escaped character in the payload
				if (response2[i]==0xAA || response2[i]==0xFF)
				{
					//LOG->Info("P3io  escape char at %d",i);
					//decrease the actual number of bytes we read since it needs to be escaped and it doesn't count to expected reply length
					bytesReadThisRound--;
					escapedBytesThisRound++;

					//do the actual unescaping later AFTER we have the full payload
				}
			}

			
		}

		//todo: bounds checking
		//LOG->Info("P3IO MEMCOP %d",num_reads);
		for(int j=0;j<bytesReadThisRound+escapedBytesThisRound;j++)
		{
			response[totalBytesRead+j]=response2[j];
		}
		//memcpy(response+totalBytesRead, response2, bytesReadThisRound);

		if ((bytesReadThisRound+escapedBytesThisRound)>0)
		{

			//LOG->Info("P3io adding %d bytes to %d",bytesReadThisRound,totalBytesRead);
			totalBytesRead+=bytesReadThisRound;
			escaped_bytes+=escapedBytesThisRound;

		}


	
	}

	//reading complete, time to unescape
	//shift all bytes in the array after this char to the left, overwriting the escaped character
	int bytesCountToEscape=totalBytesRead+escaped_bytes;
	for (int i=2;i<bytesCountToEscape;i++)
	{
		if (response[i]==0xAA || response[i]==0xFF)
		{
			bytesCountToEscape--;
			for (int j=i;j<bytesCountToEscape;j++)
			{
				response[j]=response[j+1];
			}

			//zero out the last character
			response[bytesCountToEscape]=0;

			//unescape the escaped character
			response[i]=~response[i];
		}
	}
	//LOG->Info("End P3IO get bulk  message");
	return totalBytesRead;
}


void P3IO::Reconnect()
{
	LOG->Info("Attempting to reconnect P3IO.");
	//the actual input handler will ask for a reconnect, i'd rather it happen there for more control
	/* set a message that the game loop can catch and display */
	char temp[256];
	sprintf(temp, "I/O error: %s", m_pDriver->GetError());
	//m_sInputError = ssprintf( "I/O error: %s", m_pDriver->GetError() );
	m_sInputError = m_sInputError.assign(temp);
	Close();
	m_bConnected = false;
	baud_pass=false;
	hdxb_ready=false;
	Open();
	m_sInputError = "";
}


uint8_t P3IO::checkInput(uint8_t x, uint8_t y)
{
	//there is a better way to do this but I'll optimize later, I wanted to make sure the logic was RIGHT
	uint8_t v = x;
	uint8_t w = y;

	//enable these 2 for p3io
	v = ~v;
	w = ~w;

	//printf("comparing %02X : %02X\n",v,w);
	return v & w;
}


static uint8_t p3io_init_hd_and_watchdog[21][45] = {
	{ 0xaa, 0x02, 0x00, 0x01 }, //0x01 is get version -- replies with 47 33 32 00 02 02 06 (G32)
	{ 0xaa, 0x02, 0x00, 0x2f },//?
	{ 0xaa, 0x03, 0x01, 0x27, 0x01 }, //0x27 is get cabinet type
	{ 0xaa, 0x02, 0x02, 0x31 },//coins?
	{ 0xaa, 0x02, 0x03, 0x01 }, //0x01 is get version -- replies with 47 33 32 00 02 02 06 (G32)
	{ 0xaa, 0x03, 0x04, 0x27, 0x00 }, //0x27 is get cabinet type

	//read sec plug
	{ 0xaa, 0x2b, 0x05, 0x25, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x06, 0x25, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x07, 0x25, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x08, 0x25, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x09, 0x25, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x0a, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x0b, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x0c, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x0d, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xaa, 0x2b, 0x0e, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	//end read sec plug
	
	{ 0xaa, 0x03, 0x0f, 0x05, 0x00 },//these next 4 clearly set some parameters. no idea what they are
	{ 0xaa, 0x03, 0x00, 0x2b, 0x01 },
	{ 0xaa, 0x03, 0x01, 0x29, 0x05 },
	{ 0xaa, 0x03, 0x02, 0x05, 0x30 }, //is thi sa 48 second watch dog?
	{ 0xaa, 0x03, 0x03, 0x27, 0x00 }, //0x27 is get cabinet type
};
const unsigned NUM_P3IO_INIT_MESSAGES = ARRAYLEN(p3io_init_hd_and_watchdog);

//the watchdog application needs to be running on the pc I think.
//On a nerfed pcb this likely wont do anything but on an  real cabinet,
//if you don't send a keep alive... KABOOM!
//Also init HD controls. always do so. Harmless to SD cabinets.
void P3IO::InitHDAndWatchDog()
{
	for (int i = 0; i<NUM_P3IO_INIT_MESSAGES; i++)
	{
		WriteToBulkWithExpectedReply(p3io_init_hd_and_watchdog[i], true);
		usleep(16666);
	}
	

	// now lets flush any open reads:
	FlushBulkReadBuffer();
}

void P3IO::FlushBulkReadBuffer()
{
	uint8_t response3[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	LOG->Info("P3IO Flushing Read buffer. Expect it to give up");
	bulk_reply_size=GetResponseFromBulk(response3, 3, true); //do a potentially fragmented read
}