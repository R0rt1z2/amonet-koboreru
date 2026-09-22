#include <platform.h>

const struct gpio_ies_group gpio_ies_map[] = {
    {   0,   4, 0,  1 }, {   5,   9, 0,  2 }, {  10,  13, 0, 10 },
    {  14,  15, 1, 10 }, {  16,  16, 0,  0 }, {  17,  17, 2,  2 },
    {  18,  21, 1,  3 }, {  29,  32, 0,  3 }, {  33,  33, 0,  4 },
    {  34,  36, 0,  5 }, {  37,  38, 0,  6 }, {  40,  41, 0,  9 },
    {  42,  42, 1,  0 }, {  43,  44, 0, 11 }, {  45,  46, 0, 12 },
    {  79,  82, 1, 15 }, {  83,  83, 2,  0 }, {  84,  85, 2,  1 },
    {  86,  91, 2,  2 }, {  92,  92, 0, 13 }, {  93,  95, 0, 14 },
    {  96,  99, 0, 15 }, { 106, 107, 1,  4 }, { 108, 112, 1,  1 },
    { 113, 116, 1,  2 }, { 117, 118, 1,  5 }, { 119, 124, 1,  6 },
    { 125, 126, 1,  7 }, { 127, 127, 1,  0 }, { 128, 128, 2,  8 },
    { 129, 130, 2,  9 }, { 131, 132, 2,  8 }, { 133, 134, 0,  8 },
    /* 22..28, 39, 47..78 and 100..105 have no IES bit; the MSDC pads in
     * those gaps are driven through their own msdcN_ctrl registers. */
};

typedef char gpio_ies_map_len_check[
    (sizeof(gpio_ies_map) / sizeof(gpio_ies_map[0]) == GPIO_IES_MAP_LEN) ? 1 : -1];

const struct gpio_pupd_group gpio_spec_pupd[] = {
    { 119, 121, 0xe00, 2 },    /* KROW0..2, kpad_ctrl[0] */
    { 122, 124, 0xe10, 2 },    /* KCOL0..2, kpad_ctrl[1] */
};

typedef char gpio_spec_pupd_len_check[
    (sizeof(gpio_spec_pupd) / sizeof(gpio_spec_pupd[0]) == GPIO_SPEC_PUPD_LEN) ? 1 : -1];
