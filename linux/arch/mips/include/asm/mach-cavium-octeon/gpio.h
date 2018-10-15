#ifndef __ASM_MACH_CAVIUM_OCTEON_GPIO_H
#define __ASM_MACH_CAVIUM_OCTEON_GPIO_H

/* SG legacy functional interface */
#define OCTEON_GPIO_INPUT       0x00
#define OCTEON_GPIO_OUTPUT      0x01
#define OCTEON_GPIO_INPUT_XOR   0x02
#define OCTEON_GPIO_INTERRUPT   0x04
#define OCTEON_GPIO_INT_LEVEL   0x00
#define OCTEON_GPIO_INT_EDGE    0x08

void octeon_gpio_raw_config(int line, int type);
unsigned long octeon_gpio_raw_read(void);
void octeon_gpio_raw_set(unsigned long bits);
void octeon_gpio_raw_clear(unsigned long bits);
void octeon_gpio_raw_interrupt_ack(int line);

#endif /* __ASM_MACH_GENERIC_GPIO_H */
