#ifndef INPUT_HANDLER_WIN32_RTIO_H
#define INPUT_HANDLER_WIN32_RTIO_H

#include "InputHandler.h"
#include "RageThreads.h"

#include <windows.h>
#include <string>
#include <vector>

typedef struct {
	bool P1_PadUp;
	bool P1_PadDown;
	bool P1_PadLeft;
	bool P1_PadRight;
	bool P2_PadUp;
	bool P2_PadDown;
	bool P2_PadLeft;
	bool P2_PadRight;
	bool P1_MenuUp;
	bool P1_MenuDown;
	bool P1_MenuLeft;
	bool P1_MenuRight;
	bool P2_MenuUp;
	bool P2_MenuDown;
	bool P2_MenuLeft;
	bool P2_MenuRight;
	bool P1_MenuStart;
	bool P2_MenuStart;
} GAME_INPUT;

typedef struct {
	int P1_InsertCoin;
	int P2_InsertCoin;
	int VolumeUp;
	int VolumeDown;
	int TestSwitch;
	int SelectSwitch;
} OPERATOR_INPUT;

class SerialDevice {
public:
	SerialDevice(int read_buffer_size = 0x1000, int write_buffer_size = 0x1000) : read_buffer_size_(read_buffer_size), write_buffer_size_(write_buffer_size) {}
	~SerialDevice();
	bool Connect(int com_number);
	void Disconnect();
	int Read(char *buffer, int buffer_size);
	int Write(const char *buffer, int buffer_size);
private:
	bool SetupPort();

	HANDLE com_handle_ = INVALID_HANDLE_VALUE;
	int read_buffer_size_;
	int write_buffer_size_;
	OVERLAPPED read_overlapped_;
	OVERLAPPED write_overlapped_;
};

class RtioDevice {
public:
	~RtioDevice();
	bool Connect();
	void Disconnect();
	bool ReadMsgs(std::vector<std::string> *msgs);
	bool WriteMsg(std::string const &msg);
private:
	int ParseMsg(char *buffer, int buffer_size);
	SerialDevice serial_;
	char read_buffer_[0x2000];
	int read_offset_ = 0;
};

class InputHandler_Win32_RTIO : public InputHandler
{
public:
	InputHandler_Win32_RTIO();
	~InputHandler_Win32_RTIO();
	void GetDevicesAndDescriptions(vector<InputDeviceInfo>& vDevicesOut);
	RString GetDeviceSpecificInputString(const DeviceInput &di);
	static int InputThread_Start(void *this_ptr);

private:
	bool Initialize();
	void InputThread();
	void HandleGameInput(const std::string &msg, RageTimer &now);
	void HandleOperatorInput(const std::string &msg, RageTimer &now);

	RtioDevice rtio_;
	RageThread input_thread_;
	bool shutdown_ = false;
	GAME_INPUT last_game_input_;
	OPERATOR_INPUT last_operator_input_;
};

#endif
