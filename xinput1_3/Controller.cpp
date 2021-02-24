#include "stdafx.h"
#include "Controller.h"

#include "DirectInput.h"

#define CLAMP(x, upper, lower) (min(upper, max(x, lower)))

std::vector<int> Controller::buttonFlags =
{
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_BACK,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB,
	XINPUT_GAMEPAD_LEFT_SHOULDER,
	XINPUT_GAMEPAD_RIGHT_SHOULDER,
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_Y
};

Controller::Controller() : type(DEVICE_TYPE::NONE), productGUID(""), instanceGUID("") {}

void Controller::AssignDIDevice(const std::string& instanceGUID, const std::string& productGUID, const std::vector<uint16_t>& mappings)
{
	this->instanceGUID = instanceGUID;
	this->productGUID = productGUID;
	this->type = static_cast<Controller::DEVICE_TYPE>(1);

	for (int i = 0; i < mappings.size(); i++)
	{
		std::wcout << "Key binding: " << mappings[i] << std::endl;
		bindings[i] = std::unique_ptr<DIBinding>(new DIButtonBinding(mappings[i], true));
	}
}

bool Controller::IsConnected()
{
	return type != DEVICE_TYPE::NONE;
}

HRESULT Controller::Acquire()
{
	return true;
}

HRESULT Controller::Unacquire()
{
	return true;
}

XINPUT_GAMEPAD Controller::GetState()
{
	XINPUT_GAMEPAD XInputState = { 0 };
	DIJOYSTATE2 joystate = { 0 };

	for (auto& binding : bindings)
		if(binding)
			binding->Update(joystate);

	ANALOG_SUBVALUES subValues[2];

	for (int i = 0; i < bindings.size(); i++)
	{
		if (!bindings[i])
			continue;
	
		//XBOX Buttons
		if (i < 14)
		{
			switch (bindings[i]->type)
			{
				case DIBinding::POV_BINDING:
				case DIBinding::BUTTON_BINDING:
				{
					DIButtonBinding* button = dynamic_cast<DIButtonBinding*>(bindings[i].get());
					DIPovBinding* pov = dynamic_cast<DIPovBinding*>(bindings[i].get());

					if ((pov && pov->GetState()) || (button && button->GetState()))
						XInputState.wButtons |= buttonFlags[i];
				}
				break;
			}
		}
		//Triggers
		else if (i >= 14 && i < 16)
		{
			switch (bindings[i]->type)
			{
				case DIBinding::POV_BINDING:
				case DIBinding::BUTTON_BINDING:
				{
					DIButtonBinding* button = dynamic_cast<DIButtonBinding*>(bindings[i].get());
					DIPovBinding* pov = dynamic_cast<DIPovBinding*>(bindings[i].get());

					if ((pov && pov->GetState()) || (button && button->GetState()))
					{
						if (i == 14)
							XInputState.bLeftTrigger = 255;
						else
							XInputState.bRightTrigger = 255;
					}
				}
				break;
			}
		}
		//Analogs
		else
		{
			int direction = (i - 16) % 4;
			int analogIndex = (i - 16) / 4;

			switch (bindings[i]->type)
			{
				case DIBinding::BUTTON_BINDING:
				{
					DIButtonBinding* button = dynamic_cast<DIButtonBinding*>(bindings[i].get());

					if (direction == 0)
						subValues[analogIndex].positiveY = button->GetState() ? 32768 : 0;
					else if (direction == 1)
						subValues[analogIndex].negativeY = button->GetState() ? -32768 : 0;
					else if (direction == 2)
						subValues[analogIndex].negativeX = button->GetState() ? -32768 : 0;
					else if (direction == 3)
						subValues[analogIndex].positiveX = button->GetState() ? 32768 : 0;

				}
				break;

				case DIBinding::AXIS_BINDING:
				{
					DIAxisBinding* axis = dynamic_cast<DIAxisBinding*>(bindings[i].get());

					if (direction == 0)
						subValues[analogIndex].positiveY = axis->IsNegative() ? axis->GetValue() : axis->GetValue();
					else if(direction == 1)
						subValues[analogIndex].negativeY = axis->IsNegative() ? axis->GetValue() : -axis->GetValue();
					else if (direction == 2)
						subValues[analogIndex].negativeX = axis->IsNegative() ? axis->GetValue() : -axis->GetValue();
					else if (direction == 3)
						subValues[analogIndex].positiveX = axis->IsNegative() ? -axis->GetValue() : axis->GetValue();
				}
				break;
			}
		}
	}

	int LXValue = subValues[0].negativeX != 0 ? subValues[0].negativeX : subValues[0].positiveX;
	int LYValue = subValues[0].negativeY != 0 ? subValues[0].negativeY : subValues[0].positiveY;
	int RXValue = subValues[1].negativeX != 0 ? subValues[1].negativeX : subValues[1].positiveX;
	int RYValue = subValues[1].negativeY != 0 ? subValues[1].negativeY : subValues[1].positiveY;

	XInputState.sThumbLX = std::max<int>(INT16_MIN, std::min<int>(LXValue, INT16_MAX));
	XInputState.sThumbLY = std::max<int>(INT16_MIN, std::min<int>(LYValue, INT16_MAX));
	XInputState.sThumbRX = std::max<int>(INT16_MIN, std::min<int>(RXValue, INT16_MAX));
	XInputState.sThumbRY = std::max<int>(INT16_MIN, std::min<int>(RYValue, INT16_MAX));

	lastState = XInputState;

	return XInputState;
}
