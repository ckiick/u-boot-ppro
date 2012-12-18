



/* modified by Chris J. Kiick for u-boot 2013.01 */

#include <common.h>
#include <pci.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch-oxnas820/regs.h>

#include "ehci.h"

#define wmb dmb

void start_usb_clocks(void);
int start_oxnas_usb_ehci(void);
void stop_oxnas_usb_ehci(void);

void start_usb_clocks(void)
{
    printf("Start USB clocks\n");

    // Enable ref300 clock to generate 12MHz USB clock
    writel(1UL << SYS_CTRL_CKEN_REF600_BIT, SYS_CTRL_CKEN_SET_CTRL);
    writel(25 << USB_REF_300_DIVIDER, SYS_CTRL_REF300_DIV);

    // Ensure the USB block is properly reset
    writel(1UL << SYS_CTRL_RSTEN_USBHS_BIT, SYS_CTRL_RSTEN_SET_CTRL);
    wmb();
    writel(1UL << SYS_CTRL_RSTEN_USBHS_BIT, SYS_CTRL_RSTEN_CLR_CTRL);
    writel(1UL << SYS_CTRL_RSTEN_USBPHYA_BIT, SYS_CTRL_RSTEN_SET_CTRL);
    wmb();
    writel(1UL << SYS_CTRL_RSTEN_USBPHYA_BIT, SYS_CTRL_RSTEN_CLR_CTRL);
    writel(1UL << SYS_CTRL_RSTEN_USBPHYB_BIT, SYS_CTRL_RSTEN_SET_CTRL);
    wmb();
    writel(1UL << SYS_CTRL_RSTEN_USBPHYB_BIT, SYS_CTRL_RSTEN_CLR_CTRL);

    // Force the high speed clock to be generated all the time, via serial
    // programming of the USB HS PHY
    writel((2UL << SYS_CTRL_USBHSPHY_TEST_ADD) |
           (0xe0UL << SYS_CTRL_USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

    writel((1UL << SYS_CTRL_USBHSPHY_TEST_CLK) |
           (2UL << SYS_CTRL_USBHSPHY_TEST_ADD) |
           (0xe0UL << SYS_CTRL_USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

    writel((0xfUL << SYS_CTRL_USBHSPHY_TEST_ADD) |
           (0xaaUL << SYS_CTRL_USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

    writel((1UL << SYS_CTRL_USBHSPHY_TEST_CLK) |
           (0xfUL << SYS_CTRL_USBHSPHY_TEST_ADD) |
           (0xaaUL << SYS_CTRL_USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

    // Select ref300 clock to supply USB
    writel((USB_CLK_INTERNAL << SYS_CTRL_USB_CTRL_USBAPHY_CKSEL_BIT) |
           (USB_INT_CLK_REF300 << SYS_CTRL_USB_CTRL_USB_CKO_SEL_BIT), SYS_CTRL_USB_CTRL);
}

int start_oxnas_usb_ehci()
{
    unsigned long input_polarity = 0;
    unsigned long output_polarity = 0;
	unsigned long power_polarity_default=readl(SYS_CTRL_USBHSMPH_CTRL);
	unsigned usb_hs_ifg;
	u32 reg;

	printf("initialise for OX820 series USB\n");
#ifdef CONFIG_OXNAS_USB_OVERCURRENT_POLARITY_NEGATIVE
    input_polarity = ((1UL << SYS_CTRL_USBHSMPH_IP_POL_A_BIT) |
                      (1UL << SYS_CTRL_USBHSMPH_IP_POL_B_BIT));
#endif // CONFIG_OXNAS_USB_OVERCURRENT_POLARITY_NEGATIVE

#ifdef CONFIG_OXNAS_USB_POWER_SWITCH_POLARITY_NEGATIVE
    output_polarity = ((1UL << SYS_CTRL_USBHSMPH_OP_POL_A_BIT) |
                       (1UL << SYS_CTRL_USBHSMPH_OP_POL_B_BIT));
#endif // CONFIG_OXNAS_USB_POWER_SWITCH_POLARITY_NEGATIVE

	power_polarity_default &= ~0xf;
	usb_hs_ifg = (power_polarity_default >> 25) & 0x3f;
	usb_hs_ifg += 12;
	power_polarity_default &= ~(0x3f << 25);
	power_polarity_default |= (usb_hs_ifg << 25);
	power_polarity_default |= (input_polarity & 0x3);
	power_polarity_default |= (output_polarity & ( 0x3 <<2));

	writel(power_polarity_default, SYS_CTRL_USBHSMPH_CTRL);

#ifdef CONFIG_OXNAS_USB_PORTA_POWER_CONTROL

#ifdef CONFIG_USB_PORTA_POWO_SECONDARY
	// Select USBA power output from secondary MFP function
	mask = 1UL << USBA_POWO_SEC_MFP;
    writel(readl(SYS_CTRL_SECONDARY_SEL)   |  mask, SYS_CTRL_SECONDARY_SEL);
    writel(readl(SYS_CTRL_TERTIARY_SEL)    & ~mask, SYS_CTRL_TERTIARY_SEL);
    writel(readl(SYS_CTRL_QUATERNARY_SEL)  & ~mask, SYS_CTRL_QUATERNARY_SEL);
    writel(readl(SYS_CTRL_DEBUG_SEL)       & ~mask, SYS_CTRL_DEBUG_SEL);
    writel(readl(SYS_CTRL_ALTERNATIVE_SEL) & ~mask, SYS_CTRL_ALTERNATIVE_SEL);

    // Enable output onto USBA power output secondary function
    writel(mask, GPIO_A_OUTPUT_ENABLE_SET);
#endif // CONFIG_USB_PORTA_POWO_SECONDARY

#ifdef CONFIG_USB_PORTA_POWO_TERTIARY
	// Select USBA power output from tertiary MFP function
	mask = 1UL << (USBA_POWO_TER_MFP - SYS_CTRL_NUM_PINS);
    writel(readl(SEC_CTRL_SECONDARY_SEL)   & ~mask, SEC_CTRL_SECONDARY_SEL);
    writel(readl(SEC_CTRL_TERTIARY_SEL)    |  mask, SEC_CTRL_TERTIARY_SEL);
    writel(readl(SEC_CTRL_QUATERNARY_SEL)  & ~mask, SEC_CTRL_QUATERNARY_SEL);
    writel(readl(SEC_CTRL_DEBUG_SEL)       & ~mask, SEC_CTRL_DEBUG_SEL);
    writel(readl(SEC_CTRL_ALTERNATIVE_SEL) & ~mask, SEC_CTRL_ALTERNATIVE_SEL);

    // Enable output onto USBA power output tertiary function
    writel(mask, GPIO_B_OUTPUT_ENABLE_SET);
#endif // CONFIG_USB_PORTA_POWO_TERTIARY

#ifdef CONFIG_USB_PORTA_OVERI_SECONDARY
	// Select USBA overcurrent from secondary MFP function
	mask = 1UL << USBA_OVERI_SEC_MFP;
    writel(readl(SYS_CTRL_SECONDARY_SEL)   |  mask, SYS_CTRL_SECONDARY_SEL);
    writel(readl(SYS_CTRL_TERTIARY_SEL)    & ~mask, SYS_CTRL_TERTIARY_SEL);
    writel(readl(SYS_CTRL_QUATERNARY_SEL)  & ~mask, SYS_CTRL_QUATERNARY_SEL);
    writel(readl(SYS_CTRL_DEBUG_SEL)       & ~mask, SYS_CTRL_DEBUG_SEL);
    writel(readl(SYS_CTRL_ALTERNATIVE_SEL) & ~mask, SYS_CTRL_ALTERNATIVE_SEL);

    // Enable input from USBA secondary overcurrent function
    writel(mask, GPIO_A_OUTPUT_ENABLE_CLEAR);
#endif // CONFIG_USB_PORTA_OVERI_SECONDARY

#ifdef CONFIG_USB_PORTA_OVERI_TERTIARY
	// Select USBA overcurrent from tertiary MFP function
	mask = 1UL << (USBA_OVERI_TER_MFP - SYS_CTRL_NUM_PINS);
    writel(readl(SEC_CTRL_SECONDARY_SEL)   & ~mask, SEC_CTRL_SECONDARY_SEL);
    writel(readl(SEC_CTRL_TERTIARY_SEL)    |  mask, SEC_CTRL_TERTIARY_SEL);
    writel(readl(SEC_CTRL_QUATERNARY_SEL)  & ~mask, SEC_CTRL_QUATERNARY_SEL);
    writel(readl(SEC_CTRL_DEBUG_SEL)       & ~mask, SEC_CTRL_DEBUG_SEL);
    writel(readl(SEC_CTRL_ALTERNATIVE_SEL) & ~mask, SEC_CTRL_ALTERNATIVE_SEL);

    // Enable input from USBA tertiary overcurrent function
    writel(mask, GPIO_B_OUTPUT_ENABLE_CLEAR);
#endif // CONFIG_USB_PORTA_OVERI_TERTIARY

#endif // CONFIG_OXNAS_USB_PORTA_POWER_CONTROL

#ifdef CONFIG_OXNAS_USB_PORTB_POWER_CONTROL

#ifdef CONFIG_USB_PORTB_POWO_SECONDARY
	// Select USBB power output from secondary MFP function
	mask = 1UL << USBB_POWO_SEC_MFP;
    writel(readl(SYS_CTRL_SECONDARY_SEL)   |  mask, SYS_CTRL_SECONDARY_SEL);
    writel(readl(SYS_CTRL_TERTIARY_SEL)    & ~mask, SYS_CTRL_TERTIARY_SEL);
    writel(readl(SYS_CTRL_QUATERNARY_SEL)  & ~mask, SYS_CTRL_QUATERNARY_SEL);
    writel(readl(SYS_CTRL_DEBUG_SEL)       & ~mask, SYS_CTRL_DEBUG_SEL);
    writel(readl(SYS_CTRL_ALTERNATIVE_SEL) & ~mask, SYS_CTRL_ALTERNATIVE_SEL);

    // Enable output onto USBB power output secondary function
    writel(mask, GPIO_A_OUTPUT_ENABLE_SET);
#endif // CONFIG_USB_PORTB_POWO_SECONDARY

#ifdef CONFIG_USB_PORTB_POWO_TERTIARY
	// Select USBB power output from tertiary MFP function
	mask = 1UL << USBB_POWO_TER_MFP;
    writel(readl(SYS_CTRL_SECONDARY_SEL)   & ~mask, SYS_CTRL_SECONDARY_SEL);
    writel(readl(SYS_CTRL_TERTIARY_SEL)    |  mask, SYS_CTRL_TERTIARY_SEL);
    writel(readl(SYS_CTRL_QUATERNARY_SEL)  & ~mask, SYS_CTRL_QUATERNARY_SEL);
    writel(readl(SYS_CTRL_DEBUG_SEL)       & ~mask, SYS_CTRL_DEBUG_SEL);
    writel(readl(SYS_CTRL_ALTERNATIVE_SEL) & ~mask, SYS_CTRL_ALTERNATIVE_SEL);

    // Enable output onto USBB power output tertiary function
    writel(mask, GPIO_A_OUTPUT_ENABLE_SET);
#endif // CONFIG_USB_PORTB_POWO_TERTIARY

#ifdef CONFIG_USB_PORTB_OVERI_SECONDARY
	// Select USBB overcurrent from secondary MFP function
	mask = 1UL << USBB_OVERI_SEC_MFP;
    writel(readl(SYS_CTRL_SECONDARY_SEL)   |  mask, SYS_CTRL_SECONDARY_SEL);
    writel(readl(SYS_CTRL_TERTIARY_SEL)    & ~mask, SYS_CTRL_TERTIARY_SEL);
    writel(readl(SYS_CTRL_QUATERNARY_SEL)  & ~mask, SYS_CTRL_QUATERNARY_SEL);
    writel(readl(SYS_CTRL_DEBUG_SEL)       & ~mask, SYS_CTRL_DEBUG_SEL);
    writel(readl(SYS_CTRL_ALTERNATIVE_SEL) & ~mask, SYS_CTRL_ALTERNATIVE_SEL);

    // Enable input from USBB secondary overcurrent function
    writel(mask, GPIO_A_OUTPUT_ENABLE_CLEAR);
#endif // CONFIG_USB_PORTB_OVERI_SECONDARY

#ifdef CONFIG_USB_PORTB_OVERI_TERTIARY
	// Select USBB overcurrent from tertiary MFP function
	mask = 1UL << USBB_OVERI_TER_MFP;
    writel(readl(SYS_CTRL_SECONDARY_SEL)   & ~mask, SYS_CTRL_SECONDARY_SEL);
    writel(readl(SYS_CTRL_TERTIARY_SEL)    |  mask, SYS_CTRL_TERTIARY_SEL);
    writel(readl(SYS_CTRL_QUATERNARY_SEL)  & ~mask, SYS_CTRL_QUATERNARY_SEL);
    writel(readl(SYS_CTRL_DEBUG_SEL)       & ~mask, SYS_CTRL_DEBUG_SEL);
    writel(readl(SYS_CTRL_ALTERNATIVE_SEL) & ~mask, SYS_CTRL_ALTERNATIVE_SEL);

    // Enable input from USBB tertiary overcurrent function
    writel(mask, GPIO_A_OUTPUT_ENABLE_CLEAR);
#endif // CONFIG_USB_PORTB_OVERI_TERTIARY

#endif // CONFIG_OXNAS_USB_PORTB_POWER_CONTROL

	// USB host and device must coordinate clock control
	start_usb_clocks();

	// Configure USB PHYA as a host
	reg = readl(SYS_CTRL_USB_CTRL);
	reg &= ~(1UL << SYS_CTRL_USB_CTRL_USBAMUX_DEVICE_BIT);
	writel(reg, SYS_CTRL_USB_CTRL); 

    // Enable the clock to the USB block
    writel(1UL << SYS_CTRL_CKEN_USBHS_BIT, SYS_CTRL_CKEN_SET_CTRL);
    wmb();

	return 0;
}

void stop_oxnas_usb_ehci()
{
	// put usb core into reset
    writel(1UL << SYS_CTRL_RSTEN_USBHS_BIT, SYS_CTRL_RSTEN_SET_CTRL);

    // Disable the clock to the USB block
    writel(1UL << SYS_CTRL_CKEN_USBHS_BIT, SYS_CTRL_CKEN_CLR_CTRL);
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	start_oxnas_usb_ehci();

	hccr = (struct ehci_hccr *)USBHOST_BASE;
	hcor = (struct ehci_hcor *)(USBHOST_BASE + 0x100);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
    stop_oxnas_usb_ehci();
	return 0;
}
