#pragma once

#include <Xinput.h>
#include "DIBinding.h"

class Controller
{

public:

	typedef enum
	{
		NONE,
		KEYBOARD
	} DEVICE_TYPE;

	Controller();
	~Controller() = default;

	void AssignDIDevice(const std::string& instanceGUID, const std::string& productGUID, const std::vector<uint16_t>& mappings);
	bool IsConnected();
	HRESULT Acquire();
	HRESULT Unacquire();

	XINPUT_GAMEPAD GetState();
	DEVICE_TYPE GetType() { return type; };

private:

	typedef struct
	{
		int negativeX;
		int positiveX;
		int negativeY;
		int positiveY;
	} ANALOG_SUBVALUES;

	std::array<std::unique_ptr<DIBinding>, 24> bindings;
	std::string instanceGUID;
	std::string productGUID;
	XINPUT_GAMEPAD gamepad;
	DEVICE_TYPE type;
	LPDIRECTINPUTDEVICE8 DIDevice;
	XINPUT_GAMEPAD lastState;

	static std::vector<int> buttonFlags;
};

