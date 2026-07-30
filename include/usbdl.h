#pragma once

struct string_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;         /* 0x03 */
    uint16_t wData[];
}; 

#define USB_MANUFACTURER       "PWNED"
#define USB_PRODUCT_NAME       "MT8163 Preloader"
#define USB_CONFIGURATION_STR  "USB CDC ACM for preloader"
#define USB_DATA_INTERFACE_STR "CDC ACM Data Interface"
#define USB_COMM_INTERFACE_STR "CDC ACM Communication Interface"
#define USB_ISERIAL_STR         "MT-0123456789ABCDEF"

void enter_usbdl(void);
void setup_usb_descriptors(void);
