/* USBDriver_Macros: standard USB request code macros. */

#ifndef USBDRIVER_MACROS_H
#define USBDRIVER_MACROS_H

#define USB_DIR_OUT	0x00
#define USB_DIR_IN	0x80

#define USB_TYPE_STANDARD	(0x00 << 5)
#define USB_TYPE_CLASS		(0x01 << 5)
#define USB_TYPE_VENDOR		(0x02 << 5)
#define USB_TYPE_RESERVED	(0x03 << 5)

#define USB_RECIP_DEVICE	0x00
#define USB_RECIP_INTERFACE	0x01
#define USB_RECIP_ENDPOINT	0x02
#define USB_RECIP_OTHER		0x03


#endif // USBDRIVER_MACROS_H
