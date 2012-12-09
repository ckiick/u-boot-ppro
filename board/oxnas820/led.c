
/* support for the on-board led. */

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch-oxnas820/regs.h>
#include <asm/io.h>

#if defined(CONFIG_STATUS_LED) || defined(CONFIG_CMD_LED)

#include <status_led.h>
/*
 * base addrs from regs.h. OE= output enable.
 * GPIO_2_OE, GPIO_2_SET_OE, GPIO_2_CLR_OE, GPIO_2_OUT, GPIO_2_SET, GPIO_2_CLR
 */

inline void __led_init(led_id_t mask, int state)
{
	if (state == STATUS_LED_OFF) {
		writel(mask, GPIO_2_SET_OE);
	} else {
		writel(mask, GPIO_2_CLR_OE);
	}
}

inline void __led_set(led_id_t mask, int state)
{
	if (state == STATUS_LED_OFF) {
		writel(mask, GPIO_2_CLR_OE);
	} else {
		writel(mask, GPIO_2_SET_OE);
	}
}

inline void __led_toggle(led_id_t mask)
{
	unsigned long oldstate;

	oldstate = readl(GPIO_2_OUT);

	if (oldstate & mask) {
		writel(mask, GPIO_2_CLR_OE);
	} else {
		writel(mask, GPIO_2_SET_OE);
	}
}

#endif
