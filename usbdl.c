#include <device.h>
#include <libc/string.h>
#include <mmio.h>
#include <preloader.h>
#include <usbdl.h>

#include <drivers/timer.h>

#define USB_DESCRIPTOR_TYPE_STRING 0x03

static uint8_t language[4] = { 4, USB_DESCRIPTOR_TYPE_STRING, 0x9, 0x4 };
static uint8_t manufacturer[2 + 2 * (sizeof (USB_MANUFACTURER) - 1)];
static uint8_t product[2 + 2 * (sizeof (USB_PRODUCT_NAME) - 1)];
static uint8_t configuration[2 + 2 * (sizeof (USB_CONFIGURATION_STR) - 1)];
static uint8_t dataInterface[2 + 2 * (sizeof (USB_DATA_INTERFACE_STR) - 1)];
static uint8_t commInterface[2 + 2 * (sizeof (USB_COMM_INTERFACE_STR) - 1)];
static uint8_t iserial[2 + 2 * (sizeof (USB_ISERIAL_STR) - 1)];

static struct string_descriptor *usb_string_table[] = {
    (struct string_descriptor *) language,
    (struct string_descriptor *) manufacturer,
    (struct string_descriptor *) product,
    (struct string_descriptor *) configuration,
    (struct string_descriptor *) dataInterface,
    (struct string_descriptor *) commInterface,
    (struct string_descriptor *) iserial,
};

static inline void str2wide(char *str, uint16_t *wide)
{
    unsigned int i;

    for (i = 0; i < strlen(str) && str[i]; i++)
        wide[i] = (uint16_t) str[i];
}

void setup_usb_descriptors(void)
{
    struct string_descriptor *string;

    /* Setup descriptor table */
    string = (struct string_descriptor *) manufacturer;
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    string->bLength = sizeof(manufacturer);
    str2wide(USB_MANUFACTURER, string->wData);

    string = (struct string_descriptor *) product;
    string->bLength = sizeof(product);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide(USB_PRODUCT_NAME, string->wData);

    string = (struct string_descriptor *) configuration;
    string->bLength = sizeof(configuration);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide(USB_CONFIGURATION_STR, string->wData);

    string = (struct string_descriptor *) dataInterface;
    string->bLength = sizeof(dataInterface);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide(USB_DATA_INTERFACE_STR, string->wData);

    string = (struct string_descriptor *) commInterface;
    string->bLength = sizeof(commInterface);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide(USB_COMM_INTERFACE_STR, string->wData);

    /* Overwrite the original descriptor table with ours */
    writel((uint32_t)usb_string_table, (void*)USB_STRING_TABLE_ADDR);
}

void enter_usbdl(void)
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
