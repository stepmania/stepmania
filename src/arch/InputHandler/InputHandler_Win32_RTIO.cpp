// This input handler is meant to be used with Raw Thrills DDR cabinets, such
// as the US arcade release of DDR X2.

#include "global.h"
#include "InputHandler_Win32_RTIO.h"
#include "RageLog.h"
#include "RageInputDevice.h"

#include <algorithm>
#include <vector>


// The coin counter won't accept an increment command immediately after acking
// an older increment command. This delay is the minimum amount of time to wait
// between receiving an ack for an increment command and sending a new
// increment command.
inline constexpr float COUNTER_MINIMUM_SEND_DELAY = 0.15f;

// The longest amount of time to wait for an ack to the increment command
// before moving on with the coin counter increment sequence.
inline constexpr float COUNTER_MAXIMUM_RECV_DELAY = 3.0f;

// If RTIO does not initialize within this amount of time, exit the RTIO input
// loop to free resources.
inline constexpr float RTIO_INIT_TIME_MAX = 10.0f;

// If there are this many failures, something has most likely gone very wrong,
// so just exit the RTIO input loop.
inline constexpr int RTIO_MAX_READ_FAILURES = 50;

REGISTER_INPUT_HANDLER_CLASS2(Rtio, Win32_RTIO);

InputHandler_Win32_RTIO::InputHandler_Win32_RTIO()
{
	LOG->Trace("RTIO: Start");

	if (!Initialize()) {
		return;
	}

	input_thread_.SetName("RTIO thread");
	input_thread_.Create(InputThread_Start, this);
}

InputHandler_Win32_RTIO::~InputHandler_Win32_RTIO()
{
	if (input_thread_.IsCreated())
	{
		shutdown_ = true;
		LOG->Trace("RTIO: Shutting down RTIO thread");
		input_thread_.Wait();
		LOG->Info("RTIO: Thread shut down");
	}

	rtio_.Disconnect();
}

void InputHandler_Win32_RTIO::GetDevicesAndDescriptions(std::vector<InputDeviceInfo>& vDevicesOut)
{
	// We use a joystick device so we can get automatic input mapping
	vDevicesOut.push_back(InputDeviceInfo(InputDevice(DEVICE_JOY1), "Raw Thrills I/O"));
}

RString InputHandler_Win32_RTIO::GetDeviceSpecificInputString(const DeviceInput &di)
{
	switch (di.button)
	{
	case JOY_BUTTON_1:  return "P1 Pad Left";
	case JOY_BUTTON_2:  return "P1 Pad Down";
	case JOY_BUTTON_3:  return "P1 Pad Up";
	case JOY_BUTTON_4:  return "P1 Pad Right";
	case JOY_BUTTON_5:  return "P1 Menu Left";
	case JOY_BUTTON_6:  return "P1 Menu Down";
	case JOY_BUTTON_7:  return "P1 Menu Up";
	case JOY_BUTTON_8:  return "P1 Menu Right";
	case JOY_BUTTON_9:  return "P1 Menu Start";
	case JOY_BUTTON_10: return "P2 Pad Left";
	case JOY_BUTTON_11: return "P2 Pad Down";
	case JOY_BUTTON_12: return "P2 Pad Up";
	case JOY_BUTTON_13: return "P2 Pad Right";
	case JOY_BUTTON_14: return "P2 Menu Left";
	case JOY_BUTTON_15: return "P2 Menu Down";
	case JOY_BUTTON_16: return "P2 Menu Up";
	case JOY_BUTTON_17: return "P2 Menu Right";
	case JOY_BUTTON_18: return "P2 Menu Start";
	case JOY_BUTTON_19: return "Test Switch";
	case JOY_BUTTON_20: return "Service Switch";
	case JOY_BUTTON_21: return "P1 Coin Slot";
	case JOY_BUTTON_22: return "P2 Coin Slot";
	case JOY_BUTTON_23: return "Volume Down";
	case JOY_BUTTON_24: return "Volume Up";
	}

	return InputHandler::GetDeviceSpecificInputString(di);
}

bool InputHandler_Win32_RTIO::Initialize() {
	if (!rtio_.Connect()) {
		LOG->Warn("RTIO: Initialize: Cannot connect to COM1");
		return false;
	}
	// The following messages are not required to receive game/operator inputs,
	// but they replicate the initialization sequence in DDR. We'll use the
	// response from the version message to determine that the board is good.
	if (!rtio_.WriteMsg("C0000")) {
		LOG->Warn("RTIO: Initialize: Cannot write init message");
		return false;
	}
	if (!rtio_.WriteMsg("v")) {
		LOG->Warn("RTIO: Initialize: Cannot write version message");
		return false;
	}
	if (!rtio_.WriteMsg("D0020008")) {
		LOG->Warn("RTIO: Initialize: Cannot send dongle message (1/6)");
		return false;
	}
	if (!rtio_.WriteMsg("D0010008")) {
		LOG->Warn("RTIO: Initialize: Cannot send dongle message (2/6)");
		return false;
	}
	if (!rtio_.WriteMsg("D0000020")) {
		LOG->Warn("RTIO: Initialize: Cannot send dongle message (3/6)");
		return false;
	}
	if (!rtio_.WriteMsg("D1020008")) {
		LOG->Warn("RTIO: Initialize: Cannot send dongle message (4/6)");
		return false;
	}
	if (!rtio_.WriteMsg("D1010008")) {
		LOG->Warn("RTIO: Initialize: Cannot send dongle message (5/6)");
		return false;
	}
	if (!rtio_.WriteMsg("D1000020")) {
		LOG->Warn("RTIO: Initialize: Cannot send dongle message (6/6)");
		return false;
	}
	return true;
}

int InputHandler_Win32_RTIO::InputThread_Start(void *this_ptr)
{
	((InputHandler_Win32_RTIO *)this_ptr)->InputThread();
	return 0;
}

void InputHandler_Win32_RTIO::InputThread()
{
	RageTimer start_time;
	std::vector<std::string> msgs;
	int read_failures = 0;

	while (!shutdown_) {
		if (!rtio_.ReadMsgs(&msgs)) {
			read_failures++;
			LOG->Warn("RTIO: HandlerLoop: Failed to read (%d failures)", read_failures);
			if (read_failures == RTIO_MAX_READ_FAILURES) {
				return;
			}
			continue;
		}

		RageTimer now;

		if (!initialized_ && now - start_time > RTIO_INIT_TIME_MAX) {
			LOG->Warn("RTIO: Device failed to initialize; exiting input loop");
			return;
		}

		for (auto msg : msgs) {
			if (msg.length() < 1) continue;
			if (msg[0] == 'c') {
				LOG->Trace("RTIO: Received init ack: %s", msg.c_str());
				continue;
			}
			if (msg[0] == 'v') {
				LOG->Trace("RTIO: Received version response: %s", msg.c_str());
				initialized_ = true;
				continue;
			}
			if (msg[0] == 'd') {
				LOG->Trace("RTIO: Received dongle response: %s", msg.c_str());
				continue;
			}
			if (msg[0] == 'T' && msg.length() == 7) {
				HandleGameInput(msg, now);
				continue;
			}
			if (msg[0] == 'S' && msg.length() == 11) {
				HandleOperatorInput(msg, now);
				continue;
			}
			if (msg[0] == 'h' && msg.length() == 3) {
				HandleCounterAck(msg);
				continue;
			}
			LOG->Warn("RTIO: Unrecognized response: %s", msg.c_str());
		}

		if (counter_cycles_pending_ > 0) {
			switch (counter_state_) {
			case COUNTER_STATE_SEND_1:
				if (last_counter_recv_.Ago() > COUNTER_MINIMUM_SEND_DELAY) {
					if (!rtio_.WriteMsg("H10")) {
						LOG->Warn("RTIO: Could not send coin counter increment (1/2)");
					}
					counter_state_ = COUNTER_STATE_RECV_1;
					last_counter_send_.Touch();
				}
				break;
			case COUNTER_STATE_SEND_2:
				if (last_counter_recv_.Ago() > COUNTER_MINIMUM_SEND_DELAY) {
					if (!rtio_.WriteMsg("H00")) {
						LOG->Warn("RTIO: Could not send coin counter increment (2/2)");
					}
					counter_state_ = COUNTER_STATE_RECV_2;
					last_counter_send_.Touch();
				}
				break;
			}

			if (last_counter_send_.Ago() > COUNTER_MAXIMUM_RECV_DELAY) {
				switch (counter_state_) {
				case COUNTER_STATE_RECV_1:
					LOG->Warn("RTIO: Never received coin counter increment acknowledgement (1/2)");
					counter_state_ = COUNTER_STATE_SEND_2;
					break;
				case COUNTER_STATE_RECV_2:
					LOG->Warn("RTIO: Never received coin counter increment acknowledgement (2/2)");
					counter_state_ = COUNTER_STATE_SEND_1;
					counter_cycles_pending_--;
					break;
				}
			}
		}
	}
}

int HexCharToInt(char ch)
{
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	return ch - '0';
}

void InputHandler_Win32_RTIO::HandleGameInput(const std::string &msg, const RageTimer &now)
{
	InputDevice id = InputDevice(DEVICE_JOY1);

	int pad1 = HexCharToInt(msg[1]);
	int pad2 = HexCharToInt(msg[2]);
	int menu1 = HexCharToInt(msg[3]);
	int menu2 = HexCharToInt(msg[4]);
	int start1 = HexCharToInt(msg[5]);
	int start2 = HexCharToInt(msg[6]);

	GAME_INPUT input_new;
	input_new.P1_PadUp = (pad1 >> 3) & 1;
	input_new.P1_PadDown = (pad1 >> 2) & 1;
	input_new.P1_PadLeft = (pad1 >> 1) & 1;
	input_new.P1_PadRight = pad1 & 1;
	input_new.P2_PadUp = (pad2 >> 3) & 1;
	input_new.P2_PadDown = (pad2 >> 2) & 1;
	input_new.P2_PadLeft = (pad2 >> 1) & 1;
	input_new.P2_PadRight = pad2 & 1;
	input_new.P1_MenuUp = (menu1 >> 3) & 1;
	input_new.P1_MenuDown = (menu1 >> 2) & 1;
	input_new.P1_MenuLeft = (menu1 >> 1) & 1;
	input_new.P1_MenuRight = menu1 & 1;
	input_new.P2_MenuUp = (menu2 >> 3) & 1;
	input_new.P2_MenuDown = (menu2 >> 2) & 1;
	input_new.P2_MenuLeft = (menu2 >> 1) & 1;
	input_new.P2_MenuRight = menu2 & 1;
	input_new.P1_MenuStart = start1 & 1;
	input_new.P2_MenuStart = start2 & 1;

	if (input_new.P1_PadLeft != last_game_input_.P1_PadLeft) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_1, (float)input_new.P1_PadLeft, now));
	}
	if (input_new.P1_PadDown != last_game_input_.P1_PadDown) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_2, (float)input_new.P1_PadDown, now));
	}
	if (input_new.P1_PadUp != last_game_input_.P1_PadUp) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_3, (float)input_new.P1_PadUp, now));
	}
	if (input_new.P1_PadRight != last_game_input_.P1_PadRight) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_4, (float)input_new.P1_PadRight, now));
	}
	if (input_new.P1_MenuLeft != last_game_input_.P1_MenuLeft) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_5, (float)input_new.P1_MenuLeft, now));
	}
	if (input_new.P1_MenuDown != last_game_input_.P1_MenuDown) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_6, (float)input_new.P1_MenuDown, now));
	}
	if (input_new.P1_MenuUp != last_game_input_.P1_MenuUp) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_7, (float)input_new.P1_MenuUp, now));
	}
	if (input_new.P1_MenuRight != last_game_input_.P1_MenuRight) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_8, (float)input_new.P1_MenuRight, now));
	}
	if (input_new.P1_MenuStart != last_game_input_.P1_MenuStart) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_9, (float)input_new.P1_MenuStart, now));
	}
	if (input_new.P2_PadLeft != last_game_input_.P2_PadLeft) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_10, (float)input_new.P2_PadLeft, now));
	}
	if (input_new.P2_PadDown != last_game_input_.P2_PadDown) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_11, (float)input_new.P2_PadDown, now));
	}
	if (input_new.P2_PadUp != last_game_input_.P2_PadUp) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_12, (float)input_new.P2_PadUp, now));
	}
	if (input_new.P2_PadRight != last_game_input_.P2_PadRight) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_13, (float)input_new.P2_PadRight, now));
	}
	if (input_new.P2_MenuLeft != last_game_input_.P2_MenuLeft) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_14, (float)input_new.P2_MenuLeft, now));
	}
	if (input_new.P2_MenuDown != last_game_input_.P2_MenuDown) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_15, (float)input_new.P2_MenuDown, now));
	}
	if (input_new.P2_MenuUp != last_game_input_.P2_MenuUp) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_16, (float)input_new.P2_MenuUp, now));
	}
	if (input_new.P2_MenuRight != last_game_input_.P2_MenuRight) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_17, (float)input_new.P2_MenuRight, now));
	}
	if (input_new.P2_MenuStart != last_game_input_.P2_MenuStart) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_18, (float)input_new.P2_MenuStart, now));
	}
/*
	if (memcmp(&last_game_input_, &input_new, sizeof(GAME_INPUT)) != 0) {
	LOG->Trace("RTIO: P1:%d%d%d%d P2:%d%d%d%d M1:%d%d%d%d-%d M2:%d%d%d%d-%d",
	input_new.P1_PadLeft, input_new.P1_PadDown, input_new.P1_PadUp, input_new.P1_PadRight,
	input_new.P2_PadLeft, input_new.P2_PadDown, input_new.P2_PadUp, input_new.P2_PadRight,
	input_new.P1_MenuLeft, input_new.P1_MenuDown, input_new.P1_MenuUp, input_new.P1_MenuRight, input_new.P1_MenuStart,
	input_new.P2_MenuLeft, input_new.P2_MenuDown, input_new.P2_MenuUp, input_new.P2_MenuRight, input_new.P2_MenuStart);
	}
*/
	memcpy(&last_game_input_, &input_new, sizeof(GAME_INPUT));
}

void InputHandler_Win32_RTIO::HandleOperatorInput(const std::string &msg, const RageTimer &now)
{
	InputDevice id = InputDevice(DEVICE_JOY1);

	int coin1 = HexCharToInt(msg[3]);
	int coin2 = HexCharToInt(msg[4]);
	int vol_up = HexCharToInt(msg[5]);
	int vol_dn = HexCharToInt(msg[6]);
	int test = HexCharToInt(msg[7]);
	int select = HexCharToInt(msg[8]);

	OPERATOR_INPUT input_new;
	input_new.P1_InsertCoin = coin1 & 1;
	input_new.P2_InsertCoin = coin2 & 1;
	input_new.VolumeUp = vol_up & 1;
	input_new.VolumeDown = vol_dn & 1;
	input_new.TestSwitch = test & 1;
	input_new.SelectSwitch = select & 1;

	if (input_new.TestSwitch != last_operator_input_.TestSwitch) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_19, (float)input_new.TestSwitch, now));
	}
	if (input_new.SelectSwitch != last_operator_input_.SelectSwitch) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_20, (float)input_new.SelectSwitch, now));
	}
	if (input_new.P1_InsertCoin != last_operator_input_.P1_InsertCoin) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_21, (float)input_new.P1_InsertCoin, now));
		if (input_new.P1_InsertCoin) {
			counter_cycles_pending_++;
		}
	}
	if (input_new.P2_InsertCoin != last_operator_input_.P2_InsertCoin) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_22, (float)input_new.P2_InsertCoin, now));
		if (input_new.P2_InsertCoin) {
			counter_cycles_pending_++;
		}
	}
	if (input_new.VolumeDown != last_operator_input_.VolumeDown) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_23, (float)input_new.VolumeDown, now));
	}
	if (input_new.VolumeUp != last_operator_input_.VolumeUp) {
		ButtonPressed(DeviceInput(id, JOY_BUTTON_24, (float)input_new.VolumeUp, now));
	}
/*
	if (memcmp(&last_operator_input_, &input_new, sizeof(OPERATOR_INPUT)) != 0) {
	LOG->Trace("RTIO: C:%d%d V:%d%d S:%d%d",
	input_new.P1_InsertCoin, input_new.P2_InsertCoin, input_new.VolumeUp, input_new.VolumeDown, input_new.TestSwitch, input_new.SelectSwitch);
	}
*/
	memcpy(&last_operator_input_, &input_new, sizeof(OPERATOR_INPUT));
}


void InputHandler_Win32_RTIO::HandleCounterAck(const std::string &msg)
{
	int ack_num = HexCharToInt(msg[1]);

	last_counter_recv_.Touch();

	if (ack_num == 1 && counter_state_ == COUNTER_STATE_RECV_1) {
		counter_state_ = COUNTER_STATE_SEND_2;
		return;
	}

	if (ack_num == 0 && counter_state_ == COUNTER_STATE_RECV_2) {
		counter_state_ = COUNTER_STATE_SEND_1;
		counter_cycles_pending_ = std::max(counter_cycles_pending_ - 1, 0);
		return;
	}

	LOG->Warn("RTIO: Received stray coin counter increment acknowledgement: state=%d, msg=%s", counter_state_, msg.c_str());
}



RtioDevice::~RtioDevice()
{
	Disconnect();
}

bool RtioDevice::Connect()
{
	for (int i = 1; i < 16; i++) {
		if (serial_.Connect(i)) {
			return true;
		}
	}
	return false;
}

void RtioDevice::Disconnect()
{
	serial_.Disconnect();
}

// Read any available messages from the RTIO device and return them as strings
// with the prefixes/suffixes stripped.
bool RtioDevice::ReadMsgs(std::vector<std::string> *msgs)
{
	msgs->clear();

	int bytes_read = serial_.Read(&read_buffer_[read_offset_], sizeof(read_buffer_) - read_offset_);

	// Exit if there was an error
	if (bytes_read < 0) {
		return false;
	}
	// Return early if we didn't read any bytes
	if (bytes_read == 0) {
		return true;
	}

	read_offset_ += bytes_read;

	int pos = 0;
	while (pos < read_offset_) {
		int msg_size = ParseMsg(&read_buffer_[pos], read_offset_ - pos);

		if (msg_size < 0) {
			LOG->Warn("RTIO: RtioDevice: Bad msg start at offset %d (got %d); skipping invalid data", pos, read_buffer_[pos]);
			while (pos < read_offset_) {
				if (read_buffer_[pos] == '\n')
					break;
				pos++;
			}
			continue;
		}

		if (msg_size == 0) {
			break;
		}

		msgs->push_back(std::string(&read_buffer_[pos + 1], msg_size - 2));

		pos += msg_size;
	}

	memcpy(read_buffer_, &read_buffer_[pos], read_offset_ - pos);
	read_offset_ -= pos;
	return true;
}

// Received messages always begins with '\n' and end with '\r'. Find the length
// of the first message in buffer and return its size, including the '\n' and
// '\r' characters. Return -1 upon error.
int RtioDevice::ParseMsg(char *buffer, int buffer_size)
{
	if (buffer[0] != '\n') {
		return -1;
	}
	for (int i = 1; i < buffer_size; i++) {
		if (buffer[i] == '\r') {
			return i + 1;
		}
	}
	return 0;
}

// Sends a message to the RTIO device. This function adds the necessary prefix
// and suffix ('\n' and '\r', respectively).  Messages sent to RTIO also
// include a checksum, expressed in hex.
bool RtioDevice::WriteMsg(const std::string &msg)
{
	std::string buf;
	buf = '\n';
	buf += msg;
	buf += '\r';

	char checksum = 0;
	for (unsigned int i = 0; i < buf.length(); i++) {
		checksum += buf[i];
	}

	char checksum_str[3];
	snprintf(checksum_str, sizeof(checksum_str), "%02x", checksum);
	buf += checksum_str;

	int wrote = serial_.Write(buf.c_str(), buf.length());
	return wrote == buf.length();
}



SerialDevice::~SerialDevice()
{
	Disconnect();
}

bool SerialDevice::Connect(int com_number)
{
	std::string name("COM");
	name += std::to_string(com_number);

	com_handle_ = CreateFile(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, nullptr);
	if (com_handle_ == INVALID_HANDLE_VALUE) {
		LOG->Info("RTIO: SerialDevice: Connect failed on %s: %d", name.c_str(), GetLastError());
		return false;
	}

	if (!Setup()) {
		SetCommMask(com_handle_, 0);
		return false;
	}

	LOG->Info("RTIO: SerialDevice: Connect succeeded on %s", name.c_str());
	return true;
}

void SerialDevice::Disconnect()
{
	if (com_handle_ != INVALID_HANDLE_VALUE) {
		CloseHandle(read_overlapped_.hEvent);
		CloseHandle(write_overlapped_.hEvent);
		CloseHandle(com_handle_);
		com_handle_ = INVALID_HANDLE_VALUE;
	}
}

bool SerialDevice::Setup()
{
	// Set the serial device to monitor for characters in the input buffer
	if (!SetCommMask(com_handle_, EV_RXCHAR)) {
		LOG->Warn("RTIO: SerialDevice: SetCommMask failed: %d", GetLastError());
		return false;
	}

	// Set the sizes of the device's internal buffers
	if (!SetupComm(com_handle_, read_buffer_size_, write_buffer_size_)) {
		LOG->Warn("RTIO: SerialDevice: SetupComm failed: %d", GetLastError());
		return false;
	}

	// Discard all characters from the internal buffers
	if (!PurgeComm(com_handle_, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
		LOG->Warn("RTIO: SerialDevice: PurgeComm failed: %d", GetLastError());
		return false;
	}

	// Set the timeouts for read/write operations
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 10000;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 1000;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 1000;
	if (!SetCommTimeouts(com_handle_, &timeouts)) {
		LOG->Warn("RTIO: SerialDevice: SetCommTimeouts failed: %d", GetLastError());
		return false;
	}

	// Configure control settings
	DCB dcb;
	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(com_handle_, &dcb)) {
		LOG->Warn("RTIO: SerialDevice: GetCommState failed: %d", GetLastError());
		return false;
	}
	dcb.Parity = 0;
	dcb.StopBits = 0;
	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.fBinary = 1;
	dcb.fParity = 0;
	dcb.fOutxCtsFlow = 0;
	dcb.fOutxDsrFlow = 0;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	if (!SetCommState(com_handle_, &dcb)) {
		LOG->Warn("RTIO: SerialDevice: SetCommState failed: %d", GetLastError());
		return false;
	}

	// Create a manually resettable events for the OVERLAPPED structures
	read_overlapped_.hEvent = CreateEvent(nullptr, true, false, nullptr);
	if (read_overlapped_.hEvent == nullptr) {
		LOG->Warn("RTIO: SerialDevice: CreateEvent failed: %d", GetLastError());
		return false;
	}

	write_overlapped_.hEvent = CreateEvent(nullptr, true, false, nullptr);
	if (write_overlapped_.hEvent == nullptr) {
		LOG->Warn("RTIO: SerialDevice: CreateEvent failed: %d", GetLastError());
		return false;
	}

	return true;
}

void ResetOverlapped(OVERLAPPED *overlapped)
{
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->Offset = 0;
	overlapped->OffsetHigh = 0;
	ResetEvent(overlapped->hEvent);
}


// Reads up to buffer_size bytes from the serial device. Returns as fast as
// possible by only reading bytes that are already available in the queue.
int SerialDevice::Read(char *buffer, int buffer_size)
{
	DWORD errors;
	COMSTAT stat;

	// Call ClearCommError to get the number of bytes waiting to be read
	if (!ClearCommError(com_handle_, &errors, &stat)) {
		LOG->Warn("SerialDevice: ClearCommError failed: %d", GetLastError());
		return -1;
	}

	// If there are no bytes to read, exit here
	if (stat.cbInQue == 0) {
		return 0;
	}

	ResetOverlapped(&read_overlapped_);

	DWORD read_size = std::min(stat.cbInQue, (DWORD)read_buffer_size_);
	read_size = std::min(read_size, (DWORD)buffer_size);

	DWORD bytes_transferred;
	if (!ReadFile(com_handle_, buffer, read_size, &bytes_transferred, &read_overlapped_)) {
		DWORD err = GetLastError();
		if (err != ERROR_IO_PENDING) {
			LOG->Warn("RTIO: SerialDevice: ReadFile failed: %d", err);
			return -1;
		}

		// Wait for the read operation to finish
		if (!GetOverlappedResult(com_handle_, &read_overlapped_, &bytes_transferred, true)) {
			LOG->Warn("RTIO: SerialDevice: GetOverlappedResult failed on read: %d\n", GetLastError());
			return -1;
		}
	}

	return bytes_transferred;
}

int SerialDevice::Write(const char *buffer, int buffer_size)
{
	DWORD bytes_transferred;
	DWORD write_size = std::min((DWORD) buffer_size, (DWORD) write_buffer_size_);

	ResetOverlapped(&write_overlapped_);

	if (!WriteFile(com_handle_, buffer, write_size, &bytes_transferred, &write_overlapped_)) {
		DWORD err = GetLastError();
		if (err != ERROR_IO_PENDING) {
			LOG->Warn("RTIO: SerialDevice: WriteFile failed: %d", err);
			return -1;
		}

		// Wait for the write operation to finish
		if (!GetOverlappedResult(com_handle_, &write_overlapped_, &bytes_transferred, true)) {
			LOG->Warn("RTIO: SerialDevice: GetOverlappedResult failed on write: %d\n", GetLastError());
			return -1;
		}
	}

	return bytes_transferred;
}

/*
 * Contributed by x0rbl (2019). Stepmania copyright/license:
 *
 * (c) 2003-2004 Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
