/**
 * @file usart.c
 * @Brief  cm3 串口例程，截取自cm3例程，功能是流水灯+串口发送数字.
 *         在printf结尾上必须使用\n 或者fflush(stdout);清空缓存发
 *         送数据，否则后续的printf操作会将之前的数据覆盖，且在不
 *         使用前两者的方式下printf不会输出任何的数据内容。
 * @author Yangliu, 869705086@qq.com
 * @version 1.1
 * @date 2017-01-25
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <stdio.h>
#include <stdlib.h>

int _write(int fd, char *ptr, int len);

static void clock_setup(void)
{
	/* Enable GPIOB clock for LED & USARTs. */
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Enable clocks for USART1. */
	rcc_periph_clock_enable(RCC_USART1);
}

static void usart_setup(void)
{
    // enable the USART1 interrupt
    nvic_enable_irq(NVIC_USART1_IRQ);

    // Setup GPIO pins for USART1 transmit. PA9
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, 
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);


    // Setup GPIO pins for USART1 transmit.  PA10
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
            GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

	/* Setup USART1 parameters. */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    usart_enable_rx_interrupt(USART1);

	/* Finally enable the USART. */
	usart_enable(USART1);
}

static void gpio_setup(void)
{
	/* Setup GPIO pin GPIO13 on GPIO port B for LED. */ 
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO14);
}

int main(void)
{
	int i, j = 0, c = 0;

	clock_setup();
	gpio_setup();
	usart_setup();

	printf("\nStandard      printf example.\n");

	while (1)
	{

		/*gpio_toggle(GPIOB, GPIO14);         	[> LED on/off <]*/
		c = (c == 9) ? 0 : c + 1;	            /* Increment c. */

		/* Method one */
		/*printf("%d\n",c);*/

		/* Method two */
		printf("%d", 1);
		fflush(stdout);

		if ((++j % 10) == 0)	 /* Newline after line full. */
		{
			printf("\n");
		}

		for (i = 0; i < 3000; i++)  	/* Wait a bit. */
		{
			__asm__("NOP");
		}
	}

	return 0;
}


void usart1_isr(void)
{
	static uint8_t data = 'A';

	/* Check if we were called because of RXNE. */
	if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {

		/* Indicate that we got data. */
		gpio_toggle(GPIOG, GPIO14);

		/* Retrieve the data from the peripheral. */
		data = usart_recv(USART1);
        printf(data);
	} else {
        printf('    ____    \n')
    }


}

int _write(int fd, char *ptr, int len)
{
	int i = 0;

	/*
	 * write "len" of char from "ptr" to file id "fd"
	 * Return number of char written.
	 *
	* Only work for STDOUT, STDIN, and STDERR
	 */
	if (fd > 2)
	{
		return -1;
	}

	while (*ptr && (i < len))
	{
		usart_send_blocking(USART1, *ptr);
		i++;
		ptr++;
	}

	return i;
}
