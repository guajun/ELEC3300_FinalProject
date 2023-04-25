#include "Keyboard.h"

#include "usbh_hid.h"
#include "usbh_hid_keybd.h"
#include "usb_host.h"
#include <string.h>

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost)
{
    if(USBH_HID_GetDeviceType(phost) == HID_KEYBOARD)  // if the HID is Mouse
	{
		static char key;
		static HID_KEYBD_Info_TypeDef *keyboardInfo;
		keyboardInfo = USBH_HID_GetKeybdInfo(phost);  // get the info
		key = USBH_HID_GetASCIICode(keyboardInfo);  // get the key pressed
	}
}
