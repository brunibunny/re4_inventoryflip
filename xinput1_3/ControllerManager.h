#pragma once

#include "Controller.h"

#define MAX_CONTROLLERS 1

class ControllerManager
{
public:
	static void Init();
	static bool IsConnected(DWORD controllerIndex);
	static DWORD GetState(DWORD controllerIndex, XINPUT_STATE* pState);
	static DWORD GetCapabilities(DWORD controllerIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);

private:
	static std::vector<std::unique_ptr<Controller>> controllers;
	static std::mutex stateLock;

	static bool PollGamepad(int index, XINPUT_GAMEPAD* gamepad);
};

