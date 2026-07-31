#include <platform.h>

const struct gpio_ies_group gpio_ies_map[] = {
    {   0,   9, 0,  0 }, {  10,  13, 0,  1 }, {  14,  28, 0,  2 },
    {  29,  32, 0,  3 }, {  33,  33, 1, 11 }, {  34,  38, 0, 10 },
    {  39,  42, 0, 11 }, {  43,  45, 0, 12 }, {  46,  49, 0, 13 },
    {  50,  52, 1, 10 }, {  53,  56, 0, 14 }, {  57,  58, 1,  0 },
    {  59,  65, 1,  2 }, {  66,  71, 1,  3 }, {  72,  74, 1,  4 },
    {  75,  76, 0, 15 }, {  77,  78, 1,  1 }, {  79,  82, 1,  5 },
    {  83,  84, 1,  6 },
    {  85,  90, IES_UNSUPPORTED, 0 },   /* MSDC2      */
    {  91, 100, IES_UNSUPPORTED, 0 },   /* TDP/TCP    */
    { 101, 116, IES_UNSUPPORTED, 0 },   /* GPI        */
    { 117, 120, 1,  7 },
    { 121, 126, IES_UNSUPPORTED, 0 },   /* MSDC1      */
    { 127, 137, IES_UNSUPPORTED, 0 },   /* MSDC0      */
    { 138, 141, 1,  9 },
    { 142, 142, 0, 13 },
    { 143, 154, IES_UNSUPPORTED, 0 },   /* MSDC3      */
};

typedef char gpio_ies_map_len_check[
    (sizeof(gpio_ies_map) / sizeof(gpio_ies_map[0]) == GPIO_IES_MAP_LEN) ? 1 : -1];

const struct gpio_pupd_group gpio_spec_pupd[] = {
    {  33,  35, 0xd90, 2 },    /* KPROW0..2  */
    {  36,  38, 0xda0, 2 },    /* KPCOL0..2  */
    {  46,  49, 0xdb0, 2 },    /* EINT14..17 */
    { 142, 142, 0xdc0, 2 },    /* EINT21     */
};

typedef char gpio_spec_pupd_len_check[
    (sizeof(gpio_spec_pupd) / sizeof(gpio_spec_pupd[0]) == GPIO_SPEC_PUPD_LEN) ? 1 : -1];
