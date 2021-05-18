#include "Gamepad.h"

Gamepad::Gamepad()
{
    g_pDI = nullptr;
    g_pJoystick = nullptr;
    *name = '\0';
}

void Gamepad::refresh()
{
	if (Settings::gamepadDirectInput)
	{
        UpdateInputState();
	}
	else
	{
		button_unstuck = TRUE == CONTROLS::IS_CONTROL_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_Y"));
		button_flip = TRUE == CONTROLS::IS_CONTROL_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_X"));

		button_cam = TRUE == CONTROLS::IS_CONTROL_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_NEXT_CAMERA"));

		stick_left_x = CONTROLS::GET_CONTROL_NORMAL(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_AXIS_X"));
		stick_left_y = -CONTROLS::GET_CONTROL_NORMAL(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_AXIS_Y"));
		stick_right_x = CONTROLS::GET_CONTROL_NORMAL(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_RIGHT_AXIS_X"));
		stick_right_y = -CONTROLS::GET_CONTROL_NORMAL(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_RIGHT_AXIS_Y"));

        stick_right_y = Settings::gamepadInvPitch ? -stick_right_y : stick_right_y;
	}
}

HRESULT Gamepad::InitDirectInput(HWND hWnd)
{
    HRESULT hr;

    *name = '\0';

    button_unstuck = false;
    button_flip = false;
    button_cam = false;

    raw_button[0] = false;
    raw_button[1] = false;
    raw_button[2] = false;

    stick_left_x = 0.0f;
    stick_left_y = -1.0f;
    stick_right_x = 0.0f;
    stick_right_y = 0.0f;
    
    raw_stick[0] = 0.0f;
    raw_stick[1] = 0.0f;
    raw_stick[2] = 0.0f;
    raw_stick[3] = -1.0f;


    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if (FAILED(hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION,
        IID_IDirectInput8, (VOID**)&g_pDI, nullptr)))
        return hr;

    // Look for a joystick we can use.
    if (FAILED(hr = g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,
        EnumJoysticksCallback,
        this, DIEDFL_ATTACHEDONLY)))
        return hr;

    // Make sure we got a joystick
    if (!g_pJoystick)
    {
        return S_OK;
    }

    if (FAILED(hr = g_pJoystick->SetDataFormat(&c_dfDIJoystick)))
        return hr;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    if (FAILED(hr = g_pJoystick->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE |
        DISCL_BACKGROUND)))
        return hr;

    // Enumerate the joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values property for discovered axes.
    if (FAILED(hr = g_pJoystick->EnumObjects(EnumObjectsCallback,
        this, DIDFT_ALL)))
        return hr;

    return S_OK;
}

BOOL CALLBACK Gamepad::EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance,
    VOID* pContext)
{
    HRESULT hr;

    Gamepad* gamepad = reinterpret_cast<Gamepad*>(pContext);

    // Obtain an interface to the enumerated joystick.
    hr = gamepad->g_pDI->CreateDevice(pdidInstance->guidInstance, &gamepad->g_pJoystick, nullptr);

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if (FAILED(hr))
        return DIENUM_CONTINUE;

    strncpy_s(gamepad->name, pdidInstance->tszInstanceName, MAX_PATH);

    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}

BOOL CALLBACK Gamepad::EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi,
    VOID* pContext)
{
    Gamepad* gamepad = reinterpret_cast<Gamepad*>(pContext);

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if (pdidoi->dwType & DIDFT_AXIS)
    {
        DIPROPRANGE diprg;
        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        // Set the range for the axis
        if (FAILED(gamepad->g_pJoystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
            return DIENUM_STOP;

    }

    return DIENUM_CONTINUE;
}

VOID Gamepad::ReleaseDirectInput()
{
    if (g_pJoystick)
    {
        g_pJoystick->Unacquire();
        g_pJoystick->Release();
    }
    g_pJoystick = nullptr;
}

HRESULT Gamepad::UpdateInputState()
{
    HRESULT hr;

    if (!g_pJoystick)
        return S_OK;

    // Poll the device to read the current state
    hr = g_pJoystick->Poll();
    if (FAILED(hr))
    {
        // DInput is telling us that the input stream has been
        // interrupted. We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done. We
        // just re-acquire and try again.
        hr = g_pJoystick->Acquire();
        while (hr == DIERR_INPUTLOST)
            hr = g_pJoystick->Acquire();

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        // return if a fatal error is encountered
        if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED))
            return E_FAIL;

        // if another application has control of this device, we have to wait for our turn
        if (hr == DIERR_OTHERAPPHASPRIO)
            return S_OK;
    }

    // Get the input's device state
    if (FAILED(hr = g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE), &js)))
        return hr; // The device should have been acquired during the Poll()

    LONG input[] = { js.lX, js.lY, js.lZ, js.lRx, js.lRy, js.lRz,
        js.rglSlider[1], js.rglSlider[0] };
    int AxisSettings[] = { Settings::gamepadRollAxis, Settings::gamepadPitchAxis,
        Settings::gamepadYawAxis, Settings::gamepadThrottleAxis };
    bool InvSettings[] = { Settings::gamepadInvRoll, Settings::gamepadInvPitch,
        Settings::gamepadInvYaw, Settings::gamepadInvThrottle };
    int DeadbandSettings[] = { Settings::gamepadRollDeadband, Settings::gamepadPitchDeadband,
        Settings::gamepadYawDeadband, 0 };
    int CenterSettings[] = { Settings::gamepadRollCenter, Settings::gamepadPitchCenter,
        Settings::gamepadYawCenter, Settings::gamepadThrottleCenter };
    float SensitivitySettings[] = { Settings::gamepadRollSensitivity, Settings::gamepadPitchSensitivity,
        Settings::gamepadYawSensitivity, Settings::gamepadThrottleSensitivity };
    float sticks[4];

    for (int i = 0; i < 4; i++)
    {
        LONG tmp = input[AxisSettings[i] - 1];
        raw_stick[i] = tmp / 1000.0f;

        if (tmp < (CenterSettings[i] - DeadbandSettings[i]))
        {
            sticks[i] = float(tmp - CenterSettings[i] + DeadbandSettings[i]) / (1000 + CenterSettings[i] - DeadbandSettings[i]);
        }
        else if (tmp > (CenterSettings[i] + DeadbandSettings[i]))
        {
            sticks[i] = float(tmp - CenterSettings[i] - DeadbandSettings[i]) / (1000 - CenterSettings[i] - DeadbandSettings[i]);
        }
        else
            sticks[i] = 0.0f;

        sticks[i] = InvSettings[i] ? -sticks[i] * SensitivitySettings[i] : sticks[i] * SensitivitySettings[i];

        if (sticks[i] < -1.0f)
            sticks[i] = -1.0f;
        else if(sticks[i] > 1.0f)
            sticks[i] = 1.0f;
    }

    stick_right_x = sticks[0];
    stick_right_y = sticks[1];
    stick_left_x = sticks[2];
    stick_left_y = sticks[3];

    int ButtonSettings[] = { Settings::gamepadUnstuckButton, Settings::gamepadFlipButton,
        Settings::gamepadCamButton };
    bool button[3] = { false };
    static bool prev_button[3] = { false };

    for (int i = 0; i < 3; i++)
    {
        if (ButtonSettings[i] == 0)
            raw_button[i] = false;
        else if (ButtonSettings[i] <= 8)
            raw_button[i] = input[ButtonSettings[i] - 1] > 0;
        else
            raw_button[i] = js.rgbButtons[ButtonSettings[i] - 9];

        if (prev_button[i] && raw_button[i])
            button[i] = false;
        else
            prev_button[i] = button[i] = raw_button[i];
    }

    button_unstuck = button[0];
    button_flip = button[1];
    button_cam = button[2];

    return S_OK;
}