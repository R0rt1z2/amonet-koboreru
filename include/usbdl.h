#pragma once

struct string_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;         /* 0x03 */
    uint16_t wData[];
}; 

void enter_usbdl(void);
void setup_usb_descriptors(void);
