#include <debug.h>
#include <device.h>
#include <libc/string.h>
#include <mmio.h>
#include <platform.h>
#include <preloader.h>
#include <usbdl.h>

#include <drivers/timer.h>

#define USB_DESCRIPTOR_TYPE_STRING 0x03
#define MAX_USB_STRING_LEN 63
#define DESC_BUF_SIZE (2 + (2 * MAX_USB_STRING_LEN))

static uint8_t language[4] = { 4, USB_DESCRIPTOR_TYPE_STRING, 0x09, 0x04 };
static uint8_t manufacturer[DESC_BUF_SIZE];
static uint8_t product[DESC_BUF_SIZE];
static uint8_t configuration[DESC_BUF_SIZE];
static uint8_t dataInterface[DESC_BUF_SIZE];
static uint8_t commInterface[DESC_BUF_SIZE];
static uint8_t iserial[DESC_BUF_SIZE];

static struct string_descriptor *usb_string_table[USB_STR_MAX] = {
    [USB_STR_LANG_ID]        = (struct string_descriptor *) language,
    [USB_STR_MANUFACTURER]   = (struct string_descriptor *) manufacturer,
    [USB_STR_PRODUCT]        = (struct string_descriptor *) product,
    [USB_STR_CONFIGURATION]  = (struct string_descriptor *) configuration,
    [USB_STR_DATA_INTERFACE] = (struct string_descriptor *) dataInterface,
    [USB_STR_COMM_INTERFACE] = (struct string_descriptor *) commInterface,
    [USB_STR_SERIAL]         = (struct string_descriptor *) iserial,
};

static inline void str2wide(char *str, uint16_t *wide)
{
    unsigned int i;

    for (i = 0; i < strlen(str) && str[i]; i++)
        wide[i] = (uint16_t) str[i];
}

static void do_usb_handshake(void)
{
    struct bldr_command_handler handler;

    handler.priv = 0;
    handler.attr = 0;
    handler.cb = (bldr_cmd_handler_t)(BLDR_CMD_HANDLER_ADDR | 1);

    while (1) {
        usb_handshake(&handler);
        mdelay(2500);
    }
}

static void set_descriptor_string(struct string_descriptor *desc, const char *str)
{
    if (!desc || !str) return;

    size_t len = strlen(str);
    if (len > MAX_USB_STRING_LEN) {
        len = MAX_USB_STRING_LEN;
    }

    desc->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    desc->bLength = (uint8_t)(2 + (2 * len));

    for (size_t i = 0; i < len; i++) {
        desc->wData[i] = (uint16_t)(uint8_t)str[i];
    }
}

void set_usb_string(enum usb_string_index idx, const char *str)
{
    if (idx <= USB_STR_LANG_ID || idx >= USB_STR_MAX)
        return;

    set_descriptor_string(usb_string_table[idx], str);
}

void setup_usb_descriptors(void)
{
    /* Setup default values */
    set_usb_manufacturer(USB_MANUFACTURER);
    set_usb_product(USB_PRODUCT_NAME);
    set_usb_configuration(USB_CONFIGURATION_STR);
    set_usb_data_interface(USB_DATA_INTERFACE_STR);
    set_usb_comm_interface(USB_COMM_INTERFACE_STR);
    set_usb_serial(USB_ISERIAL_STR);

    /* Overwrite the original descriptor table pointer with ours */
    writel((uint32_t)usb_string_table, (void *)USB_STRING_TABLE_ADDR);
}

__attribute__((weak)) uint8_t usbdl_detect_key(void)
{
    return 0;
}

void enter_usbdl(uint8_t force)
{
    if (force || (usb_cable_in() && usbdl_detect_key())) {
        printf("Entering pwned USBDL as you wish...\n");
        do_usb_handshake();
    }
}
