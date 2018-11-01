#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <stdio.h>
#include <stdlib.h>

static c1count = 0, c2count = 0;

int _write(int fd, char *ptr, int len);

static void clock_setup(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();


	/* Enable GPIOB clock for LED & USARTs. */
	rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_AFIO);

	/* Enable clocks for USART1. */
	rcc_periph_clock_enable(RCC_USART1);

    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_TIM4);
}


static void tim_setup(void){
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM3_CH1 | GPIO_TIM3_CH2);

	TIM3_CR1 = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
    
	/* Period */
	TIM3_ARR = 65535;
	/* Prescaler */
	TIM3_PSC = 0;
	TIM3_EGR = TIM_EGR_UG;

	/* ---- */
	/* Output compare 1 mode and preload */
	TIM3_CCMR1 |= TIM_CCMR1_OC1M_PWM2 | TIM_CCMR1_OC1PE;


	/* Polarity and state */
	TIM3_CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;
	//TIM3_CCER |= TIM_CCER_CC1E;

	/* Capture compare value */
	TIM3_CCR1 = 0;

	/* ---- */
	/* Output compare 2 mode and preload */
	TIM3_CCMR1 |= TIM_CCMR1_OC2M_PWM2 | TIM_CCMR1_OC2PE;

	/* Polarity and state */
	TIM3_CCER |= TIM_CCER_CC2P | TIM_CCER_CC2E;
	//TIM3_CCER |= TIM_CCER_CC2E;

	/* Capture compare value */
	TIM3_CCR2 = 0;
    
    
	/* ---- */
	/* ARR reload enable */
	TIM3_CR1 |= TIM_CR1_ARPE;

	/* Counter enable */
	TIM3_CR1 |= TIM_CR1_CEN;


    // PB6,7,8,9
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, 
            GPIO_CNF_INPUT_FLOAT, GPIO_TIM4_CH1);
    timer_disable_counter(TIM4);
    rcc_periph_reset_pulse(RST_TIM4);
    nvic_set_prioity(NVIC_DMA1_CHANNEL3_IR);
}

static void usart_setup(void)
{
    // enable the USART1 interrupt
    nvic_enable_irq(NVIC_USART1_IRQ);

    // Setup GPIO pins for USART1 transmit. PA9   PB6
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, 
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    // Setup GPIO pins for USART1 receiver.  PA10   PB7
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
            GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

	/* Setup USART1 parameters. */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	usart_set_mode(USART1, USART_MODE_TX_RX);

    /*usart_enable_rx_interrupt(USART1);*/
    USART_CR1(USART1) |= USART_CR1_RXNEIE;

	/* Finally enable the USART. */
	usart_enable(USART1);
}

static void gpio_setup(void)
{
	/* Setup GPIO pin GPIO13 on GPIO port B for LED. */ 
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO14);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO15);
}

int main(void)
{
    int i = 0;
    int j = 2500; 
    int m;
    clock_setup();
	gpio_setup();
	/*nvic_setup();*/
    usart_setup();
    
    tim_setup();

	printf("\nStandard      printf example.\n");

	while (1)
	{
        i += j;

        if (i == 70000)
            j = -2500;
        if (i == 0)
            j = 2500;
        TIM3_CCR1 = i;
        TIM3_CCR2 = i;
        /*usart_sen(USART1, i);*/
        printf("%d\n", i);
		/*gpio_toggle(GPIOB, GPIO14);         	[> LED on/off <]*/
		//c = (c == 9) ? 0 : c + 1;	            /* Increment c. */

		/* Method one */
		/*printf("%d\n",c);*/

		/* Method two */
		//printf("%d", 1);
		//fflush(stdout);

		//if ((++j % 10) == 0)	 /* Newline after line full. */
		/*{*/
			/*printf("\n");*/
		/*}*/

        for (m = 0; m < 30000000; m++)  	/* Wait a bit.*/
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
        gpio_clear(GPIOB, GPIO14);

        /* Retrieve the data from the peripheral. */
        data = usart_recv(USART1);

        /* Enable transmit interrupt so it sends back the data. */
        USART_CR1(USART1) |= USART_CR1_TXEIE;                                                                                                                                                
    }   

    /* Check if we were called because of TXE. */
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_TXE) != 0)) {
        /* Indicate that we are sending out data. */
        gpio_set(GPIOB, GPIO14);

        /* Put data into the transmit register. */
        usart_send(USART1, data);

        /* Disable the TXE interrupt as we don't need it anymore. */
        USART_CR1(USART1) &= ~USART_CR1_TXEIE;
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
