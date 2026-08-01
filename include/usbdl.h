#pragma once

enum usb_string_index {
    USB_STR_LANG_ID = 0,
    USB_STR_MANUFACTURER,
    USB_STR_PRODUCT,
    USB_STR_CONFIGURATION,
    USB_STR_DATA_INTERFACE,
    USB_STR_COMM_INTERFACE,
    USB_STR_SERIAL,
    USB_STR_MAX
};

struct string_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;         /* 0x03 */
    uint16_t wData[];
}; 

void set_usb_string(enum usb_string_index idx, const char *str);
static inline void set_usb_manufacturer(const char *str)   { set_usb_string(USB_STR_MANUFACTURER, str); }
static inline void set_usb_product(const char *str)        { set_usb_string(USB_STR_PRODUCT, str); }
static inline void set_usb_configuration(const char *str)  { set_usb_string(USB_STR_CONFIGURATION, str); }
static inline void set_usb_data_interface(const char *str) { set_usb_string(USB_STR_DATA_INTERFACE, str); }
static inline void set_usb_comm_interface(const char *str) { set_usb_string(USB_STR_COMM_INTERFACE, str); }
static inline void set_usb_serial(const char *str)         { set_usb_string(USB_STR_SERIAL, str); }

void enter_usbdl(void);
void setup_usb_descriptors(void);
