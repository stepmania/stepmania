#include "global.h"
#include "ACIO.h"


//GIANT HACKKKKKKKKK figuring this out still but it works well enough. Just spam the hell out of this
bool ACIO::baudCheckWrapper(serial::Serial &acio_bus)
{
	baudCheck(acio_bus);
	baudCheck(acio_bus);
	baudCheck(acio_bus);
	baudCheck(acio_bus);
	return baudCheck(acio_bus);
}

bool ACIO::baudCheck(serial::Serial &acio_bus)
{
	int baudHits=0;
	int tries=0;
	uint8_t baud_check[]={0xaa};
	uint8_t baud_packet[]={0xaa};
	while(tries<500)
	{
		//LOG->Info("Baud rate try %d...\r",tries);
		if(baudHits>150)
		{
			return true;
		}
		acio_bus.available();//clear comm error
		acio_bus.write(baud_check,1);
		int r= acio_bus.read(baud_packet,1);
		if (r>0)
		{
			//LOG->Info("ACIO: Got %d bytes - %02X",r,baud_packet[0]);
			baudHits++;
		}
		else
		{
			//LOG->Info("ACIO: Nothing on the baud check!",r,baud_packet[0]);
		}
		tries++;
	}
	return false;
}

//Adds the leading AA packet start, escapes your data, calculates the checksum
//highly recommended to pass in a buffer of 256 unless you know what your doing
//expects checksum byte included in length BUT will fill it in for you
//expects the length parameter to be filled out in the packet constructor for a standard packet because
//certain packets do not conform to that standard so we need to be permisive of that
int ACIO::prep_acio_packet_for_transmission(uint8_t* acio_request, int r_length, int init_escape_offset)
{
	uint8_t acio_request2[256];
	uint8_t chksum=0;
	int len=0;
	memset(acio_request2, 0, sizeof(acio_request2));
	acio_request2[0]=0xAA;
	int escape_offset=0;
	

	
	//copy request and escape as nessecary
	for (len=0;len<r_length-1;len++)
	{
		chksum=(chksum+acio_request[len])&0xFF; // calculate checksum as we copy to buffer

		if ((acio_request[len]==0xAA || acio_request[len]==0xFF) && len >= init_escape_offset)
		{
			acio_request2[len+escape_offset+1]=0xFF;
			escape_offset++;
			acio_request2[len+escape_offset+1]=~acio_request[len];
		}
		else
		{
			acio_request2[len+escape_offset+1]=acio_request[len];
		}
	}
	acio_request2[len+escape_offset+1]=chksum; // add checksum to payload
	len=r_length+escape_offset+1;

	//give the nice people back their bastardized packet for them to fuck with
	for (int i=0; i< len; i++)
	{
		acio_request[i]=acio_request2[i];
	}

	//give them back their new length
	return len;
}


//pass a zero to data start if we are escaping a partial packet / packet fragment
int ACIO::unescape_acio_packet(uint8_t* acio_response2, int len, int data_start)
{
	uint8_t acio_response[258];
	memset(acio_response, 0, sizeof(acio_response));
	
	int j=0;
	
	//iterate through the stream we got -- we want to strip off any leading 0xAA values
	for (int i=0;i<len;i++)
	{
		if (data_start<0)
		{
			if (acio_response2[i]!=0xAA) //check for data start
			{
				continue;
			}
			else
			{
				data_start=i;
			}
		}
		
		//if message has started
		if (data_start>=0)
		{
			if (acio_response2[i]!=0xFF && acio_response2[i]!=0xAA) //check for escape byte, if not, just copy to our buffer
			{
				acio_response[j]=acio_response2[i];
				j++;
			}
			else
			{ //otherwise escape and copy to our buffer
				i++;
				acio_response[j]=~acio_response2[i];
				j++;
			}
			
		}
	}

	//we now have a good packet in acio_response, we need to copy to acio_response2 for the user
	for (int i =0; i< len; i++)
	{
		acio_response2[i]=acio_response[i];
	}

	//return the UNESCAPED length we got.. we may need to get more because we only read the expected size. If data was escaped, well, fuck. Read again.
	return j;

}


int ACIO::write_acio_packet(serial::Serial &acio_bus, uint8_t* acio_request,int r_length)
{
	int len = ACIO::prep_acio_packet_for_transmission(acio_request,r_length);
	acio_bus.available();//clear comm error
	len = acio_bus.write(acio_request,len);
	return len;
}


int ACIO::read_acio_packet(serial::Serial &acio_bus, uint8_t* acio_response)
{
	uint8_t acio_response2[258];
	int response_position = 0;

	//int expected_bytes_to_read_left_to_read = get_expected_reply_bytes_from_request() +2;

	int expected_bytes_to_read_left_to_read=5;

	while (expected_bytes_to_read_left_to_read>0)
	{
		//read data
		int bytes_read_this_round = acio_bus.read(acio_response2,expected_bytes_to_read_left_to_read);

		if (bytes_read_this_round<1)
		{
			// didnt get a acio response? terminate...
			break;
		}

		//unescape the data
		int unescaped_bytes_read_this_round = ACIO::unescape_acio_packet(acio_response2,bytes_read_this_round,(response_position==0?-1:0));

			if (unescaped_bytes_read_this_round<1)
		{
			// got a bad acio response? terminate...
			break;
		}

		//copy into the full packet
		for (int i=0;i<unescaped_bytes_read_this_round;i++)
		{
			acio_response[response_position+i]=acio_response2[i];
		}
		response_position+=unescaped_bytes_read_this_round; // this is now the full packet length
		expected_bytes_to_read_left_to_read-=unescaped_bytes_read_this_round;







		//special case -- if we got all 5 first bytes and were ready to go
		if (response_position==5 && expected_bytes_to_read_left_to_read==0)
		{
			//check what we have in the buffer
			//first check for a broadcast
			int total_size_to_check=5;

			if (acio_response[0]>=0x10)
			{
				//if broadcast, we have 2 preamble bytes and 1 checksum = 3 + (2* number of broadcasting nodes? why is this 2?) = total size
				int num_broadcasting_nodes= ((acio_response[0] >> 4) & 0x0f);
				total_size_to_check=(2*num_broadcasting_nodes) + 3;
			}
			else
			{
				//if a regular packet, we have 5 frame bytes and 1 checksum = 6 + frame[4] (payload size) = total size
				total_size_to_check=6+acio_response[4];
			}

			//our one time adjustment based on what we read.
			expected_bytes_to_read_left_to_read=total_size_to_check-response_position;
		}
		//end special case!!

	}

	return response_position;


	/*
	uint8_t acio_response2[258];
	memset(acio_response, 0, sizeof(acio_response));
	memset(acio_response2, 0, sizeof(acio_response2));
	
	int r = satellites.read(acio_response2,get_expected_reply_bytes_from_request()+2);
	int j=0;
	int data_start=-1;
	
	//iterate through the stream we got -- we want to strip off any leading 0xAA values
	for (int i=0;i<r;i++)
	{
		if (data_start<0)
		{
			if (acio_response2[i]!=0xAA) //check for data start
			{
				continue;
			}
			else
			{
				data_start=i;
			}
		}
		
		//if message has started
		if (data_start>=0)
		{
			if (acio_response2[i]!=0xFF && acio_response2[i]!=0xAA) //check for escape byte, if not, just copy to our buffer
			{
				acio_response[j]=acio_response2[i];
				j++;
			}
			else
			{ //otherwise escape and copy to our buffer
				i++;
				acio_response[j]=~acio_response2[i];
				j++;
			}
			//printf(" %02X",acio_response[j] &0xFF);
		}
	}
	return j;
	*/
}



//helper function - reads an array entry and populates 
int ACIO::assemble_init_packet_to_write(uint8_t* acio_request, uint8_t* init_line)
{
	bool isInitPacket = (!(init_line==NULL));
	int i=0;
	if (isInitPacket) // only copy if we are going from init array
	{
		for (i=0;i<2;i++)
		{
			acio_request[i]=init_line[i];
		}
	}
	uint8_t bcast=acio_request[0];
	uint8_t blen=acio_request[1];
	int msg_len=6;
	if (bcast >=0x10)
	{
		msg_len=blen+3;
	}
	else
	{

		if (isInitPacket) // only copy if we are going from init array
		{
			for (i=2;i<msg_len;i++)
			{
				acio_request[i]=init_line[i];
			}
		}

		msg_len+=acio_request[4];
	}

	if (isInitPacket) // only copy if we are going from init array
	{
		for (i;i<msg_len;i++)
		{
			acio_request[i]=init_line[i];
		}
	}
	return msg_len;
}