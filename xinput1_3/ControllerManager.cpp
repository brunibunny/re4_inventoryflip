#include "stdafx.h"
#include "ControllerManager.h"

#include <fstream>
#include "DirectInput.h"

std::vector<std::unique_ptr<Controller>> ControllerManager::controllers;

void ControllerManager::Init()
{
	if (!DirectInput::Init())
	{
		std::cout << "DirectInput failed to initialize." << std::endl;
		return;
	}

	std::cout << "DirectInput initialized." << std::endl;

	controllers.clear();

	for (int i = 0; i < 4; i++)
		controllers.emplace_back(std::make_unique<Controller>());

	int guidLength1 = 0;
	int guidLength2 = 0;
	char* instanceGUID = nullptr;
	char* productGUID = nullptr;
	std::vector<uint16_t> mappings;

	mappings.resize(24);

	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		instanceGUID = new char[guidLength1 + 1];
		instanceGUID[guidLength1] = 00000000-0000-0000-0000-000000000000;

		productGUID = new char[guidLength2 + 1];
		productGUID[guidLength2] = 00000000-0000-0000-0000-000000000000;

		mappings[20] = 327;
		mappings[21] = 335;
		mappings[22] = 338;
		mappings[23] = 329;

		controllers[i]->AssignDIDevice(instanceGUID, productGUID, mappings);
	}
}

bool ControllerManager::IsConnected(DWORD controllerIndex)
{
	return controllers[controllerIndex]->IsConnected();
}

DWORD ControllerManager::GetState(DWORD controllerIndex, XINPUT_STATE* pState)
{
	if (PollGamepad(controllerIndex, &pState->Gamepad))
	{
		pState->dwPacketNumber = GetTickCount();
		return ERROR_SUCCESS;
	}
	else
	{
		std::cout << "Failed to poll for gamepad state." << std::endl;
		return ERROR_DEVICE_UNREACHABLE;
	}
}

DWORD ControllerManager::GetCapabilities(DWORD controllerIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
{
	if (PollGamepad(controllerIndex, &pCapabilities->Gamepad))
	{
		pCapabilities->Type = XINPUT_DEVTYPE_GAMEPAD;
		pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
		return ERROR_SUCCESS;
	}
	else
	{
		std::cout << "Failed to poll for gamepad capabilities." << std::endl;
		return ERROR_DEVICE_UNREACHABLE;
	}
}

bool ControllerManager::PollGamepad(int index, XINPUT_GAMEPAD* gamepad)
{
	auto res = controllers[index]->Acquire();

	if (SUCCEEDED(res))
	{
		auto gamepadState = controllers[index]->GetState();
		std::memcpy(gamepad, &gamepadState, sizeof(XINPUT_GAMEPAD));
		res = controllers[index]->Unacquire();

		if (FAILED(res))
		{
			std::cout << "Failed to unacquire. HRESULT=0x" << std::hex << res << std::endl;
		}

		return true;
	}
	else
	{
		std::cout << "Failed to acquire. HRESULT=0x" << std::hex << res << std::endl;
		return false;
	}
}
