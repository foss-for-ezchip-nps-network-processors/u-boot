/*
 * Copyright (C) 2012 EZchip, Inc. (www.ezchip.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <asm/arch/regs.h>
#include <asm/arch/gpio.h>

struct gpio_addr {
	unsigned long mode_addr;
	unsigned long out_addr;
	unsigned long in_addr;
};

static struct gpio_addr gpio_addrs[] = {
	[PREG_EGPIO] = { PREG_EGPIO_EN_N, PREG_EGPIO_O, PREG_EGPIO_I },
	[PREG_FGPIO] = { PREG_FGPIO_EN_N, PREG_FGPIO_O, PREG_FGPIO_I },
	[PREG_GGPIO] = { PREG_GGPIO_EN_N, PREG_GGPIO_O, PREG_GGPIO_I } };

int set_gpio_mode(gpio_bank_t bank, int bit, gpio_mode_t mode)
{
	unsigned long addr = gpio_addrs[bank].mode_addr;

	WRITE_CBUS_REG_BITS(addr, mode, bit, 1);

	return 0;
}

gpio_mode_t get_gpio_mode(gpio_bank_t bank, int bit)
{
	unsigned long addr = gpio_addrs[bank].mode_addr;

	return (READ_CBUS_REG_BITS(addr, bit, 1) > 0) ? (GPIO_INPUT_MODE) : \
	(GPIO_OUTPUT_MODE);
}

int set_gpio_val(gpio_bank_t bank, int bit, unsigned long val)
{
	unsigned long addr = gpio_addrs[bank].out_addr;

	WRITE_CBUS_REG_BITS(addr, val ? 1 : 0, bit, 1);

	return 0;
}

unsigned long  get_gpio_val(gpio_bank_t bank, int bit)
{
	unsigned long addr = gpio_addrs[bank].in_addr;

	return READ_CBUS_REG_BITS(addr, bit, 1);
}

/*
 * enable gpio edge interrupt
 *
 * [in] pin  index number of the chip, start with 0 up to 255
 * [in] flag rising(0) or falling(1) edge
 * [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
void gpio_enable_edge_int(int pin, int flag, int group)
{
	group &= 7;

	unsigned ireg = GPIO_INTR_GPIO_SEL0 + (group >> 2);

	SET_CBUS_REG_MASK(ireg, pin << ((group & 3) << 3));
	SET_CBUS_REG_MASK(GPIO_INTR_EDGE_POL, ((flag << (16 + group)) | \
		(1 << group)));
}

/*
 * enable gpio level interrupt
 *
 * [in] pin  index number of the chip, start with 0 up to 255
 * [in] flag high(0) or low(1) level
 * [in] group  this interrupt belong to which interrupt group from 0 to 7
 */
void gpio_enable_level_int(int pin, int flag, int group)
{
	group &= 7;
	unsigned ireg = GPIO_INTR_GPIO_SEL0 + (group >> 2);
	SET_CBUS_REG_MASK(ireg, pin << ((group & 3) << 3));
	SET_CBUS_REG_MASK(GPIO_INTR_EDGE_POL, ((flag << (16 + group)) | \
		(0 << group)));
}

/*
 * enable gpio interrupt filter
 *
 * [in] filter from 0~7(*222ns)
 * [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
void gpio_enable_int_filter(int filter, int group)
{
	group &= 7;
	unsigned ireg = GPIO_INTR_FILTER_SEL0;
	SET_CBUS_REG_MASK(ireg, filter << (group << 2));
}

int gpio_is_valid(int number)
{
	return 1;
}

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

void gpio_free(unsigned gpio)
{
}

int gpio_direction_input(unsigned gpio)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;
	set_gpio_mode(bank, bit, GPIO_INPUT_MODE);
	printf("set gpio%d.%d input\n", bank, bit);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;

	set_gpio_val(bank, bit, value ? 1 : 0);
	set_gpio_mode(bank, bit, GPIO_OUTPUT_MODE);
	printf("set gpio%d.%d output\n", bank, bit);

	return 0;
}

void gpio_set_value(unsigned gpio, int value)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;

	set_gpio_val(bank, bit, value ? 1 : 0);
}

int gpio_get_value(unsigned gpio)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;

	return get_gpio_val(bank, bit);
}
