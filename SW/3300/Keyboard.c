#include "Keyboard.h"

#include "usbh_hid.h"
#include "usbh_hid_keybd.h"
#include "usb_host.h"
#include <string.h>


// extern HID_KEYBD_Info_TypeDef keybd_info;
// extern const uint8_t HID_KEYBRD_ShiftKey[];
// extern const uint8_t HID_KEYBRD_Codes[];
// extern const uint8_t HID_KEYBRD_Key[];

uint8_t imuMotorCtr = 0;

struct Keys keys;

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost)
{
    if(USBH_HID_GetDeviceType(phost) == HID_KEYBOARD)  // if the HID is Mouse
	{
		static char key;
		static HID_KEYBD_Info_TypeDef *keyboardInfo;
		keyboardInfo = USBH_HID_GetKeybdInfo(phost);  // get the info
		// key = USBH_HID_GetASCIICode(keyboardInfo);  // get the key pressed


		uint32_t keyBits = 0;

		for(uint8_t i = 0; i < 6; ++i)
		{
			switch (keyboardInfo->keys[i])
			{
			case KEY_Q:
				keyBits |= 1;
				break;

			case KEY_W:
				keyBits |= 1 << 1;
				break;

			case KEY_E:
				keyBits |= 1 << 2;
				break;	

			case KEY_R:
				keyBits |= 1 << 3;
				break;

			case KEY_T:
				keyBits |= 1 << 4;
				break;

			case KEY_Y:
				keyBits |= 1 << 5;
				break;

			case KEY_A:
				keyBits |= 1 << 6;
				break;

			case KEY_S:
				keyBits |= 1 << 7;
				break;

			case KEY_D:
				keyBits |= 1 << 8;
				break;

			case KEY_F:
				keyBits |= 1 << 9;
				break;

			case KEY_G:
				keyBits |= 1 << 10;
				break;

			case KEY_H:
				keyBits |= 1 << 11;
				break;

			case KEY_L:
				keyBits |= 1 << 12;
				break;
			
			default:
				break;
			}
		}

		keys.Q =  !!(keyBits & (1));
		keys.W =  !!(keyBits & (1 << 1));
		keys.E =  !!(keyBits & (1 << 2));
		keys.R =  !!(keyBits & (1 << 3));
		keys.T =  !!(keyBits & (1 << 4));
		keys.Y =  !!(keyBits & (1 << 5));
		keys.A =  !!(keyBits & (1 << 6));
		keys.S =  !!(keyBits & (1 << 7));
		keys.D =  !!(keyBits & (1 << 8));
		keys.F =  !!(keyBits & (1 << 9));
		keys.G =  !!(keyBits & (1 << 10));
		keys.H =  !!(keyBits & (1 << 11));
		keys.L =  !!(keyBits & (1 << 12));

		if(keys.L)
		{
			imuMotorCtr = !imuMotorCtr;
		}
	}



}

