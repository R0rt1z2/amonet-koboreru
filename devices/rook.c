#include <arb.h>
#include <device.h>
#include <patch.h>
#include <tee.h>

#include <drivers/gpio.h>
#include <drivers/timer.h>
#include <drivers/pmic_keys.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021B630, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x002170E0, 0xBF004628);

    // Replace the ARB check with a routine that clears the counters.
    patch_branch(0x00201934, clear_rpmb_arb);

    // Replace the TEE image loader with our own
    patch_branch(0x0020A32C, bldr_load_tee_part);
}

static uint8_t key_pressed(uint32_t pin)
{
    mt_set_gpio_mode(pin, GPIO_MODE_GPIO);
    mt_set_gpio_dir(pin, GPIO_DIR_IN);
    mt_set_gpio_ies(pin, GPIO_IES_ENABLE);
    mt_set_gpio_pull(pin, GPIO_PULL_ENABLE, GPIO_PULL_UP);

    mdelay(1);

    return mt_get_gpio_in(pin) == 0;
}

uint8_t usbdl_detect_key(void)
{
    return pmic_key_pressed(USBDL_PMIC_KEY)
        && key_pressed(VOL_DOWN_GPIO);
}
