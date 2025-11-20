#include <linux/gpio/consumer.h>

struct display_st7796s_conf {
    void (*prepare)(void*);
    void (*finish)(void*);
    struct gpio_desc *sel_st7796s;
    struct gpio_desc *cmd_st7796s;
};
