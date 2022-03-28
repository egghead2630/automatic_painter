#include"stm32l476xx.h"
#include<stdio.h>
#include<string.h>
extern int Delay_1s();
extern void fpu_enable();
float lux;								// SI unit of lightness
unsigned char cmd[128];
unsigned char msg[32]= "1\n";
int now = 0;
int state = 0;
void Systick_Config()
{
SysTick->CTRL &= 0xFFFFFFF8;	// mask 4,2,1

SysTick->CTRL |= 0x4;	// follow processor
SysTick->LOAD = (uint32_t)(400000);		// 0.1s
SysTick->VAL = 0;

SysTick->CTRL |= 0x3;		// enable CNTEN and exception enable

}
/*
void ADC1_init()
{
	// enable ADC clock // clock is default SYSCLK
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;

	// PC0 is reset as analog mode // but just in case
	GPIOC->MODER |= GPIO_MODER_MODE0;
	// set PC0 ADC input(connect analog switch to analog input)
	GPIOC->ASCR |= GPIO_ASCR_ASC0;
								// note that PC0 corresponding to ADC123 input
	ADC1->CFGR &= ~ADC_CFGR_RES;	// 12-bit resolution
//	ADC1->CFGR |= ADC_CFGR_RES_0;
	ADC1->CFGR &= ~ADC_CFGR_CONT;	// single-conversion mode // we use interrupt to read value
	ADC1->CFGR &= ~ADC_CFGR_ALIGN; // right align

	ADC123_COMMON->CCR &= ~ADC_CCR_DUAL;		// set DUAL to 0 for independent mode

	ADC123_COMMON->CCR &= ~ADC_CCR_CKMODE;
	ADC123_COMMON->CCR |= ADC_CCR_CKMODE_0;
												// now CLK = HCLK / 1;

	ADC123_COMMON->CCR &= ~ADC_CCR_PRESC;		// not adjust ADC CLK

	ADC123_COMMON->CCR &= ~ADC_CCR_MDMA;
												// no DMA
	ADC123_COMMON->CCR &= ~ADC_CCR_DELAY; // delay: 5 adc clk cycle
	ADC123_COMMON->CCR |= ADC_CCR_DELAY_0;
	ADC123_COMMON->CCR |= ADC_CCR_DELAY_2;


	ADC1->SQR1 &= ~ADC_SQR1_SQ1;
	ADC1->SQR1 |= ADC_SQR1_SQ1_0;		// we set channel 1 of ADC1 to be rank 1

	ADC1->SMPR1 &= ~ADC_SMPR1_SMP0;
	ADC1->SMPR1 |= ADC_SMPR1_SMP0_0 | ADC_SMPR1_SMP0_1;
										// set sampling time 011 24.5

	ADC1->CR &= ~ADC_CR_DEEPPWD;		// left deep-power down mode // ready to start;
	ADC1->CR |= ADC_CR_ADVREGEN;		// Voltage REGulator enable

	for(int i = 0; i < 5000; i++);		// give it some time for voltage regulator to start

	ADC1->IER |= ADC_IER_EOCIE;				// we enable the ADC to interrupt when ending regular conversion
											// namely, read value;

	NVIC_EnableIRQ(ADC1_2_IRQn);			// ADC12 interrupt enabled,for  end of conversion interrupt

	ADC1->CR |= ADC_CR_ADEN;		// enable ADC

	while(!(ADC1->ISR & ADC_ISR_ADRDY));	// wait until ADC ready

}
*/

void GPIO_init()	// PC6(Tim3): PWM for two;PA0(Tim2): PWM for third, PA5: third DIR; PB6: RX, PB7: TX; PB3: two DIR, PB4: third ENA, PB5: two ENA,
{
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;
				// define PB6 PB7 as AF_MODE for USART1_TX USART1_RX
	// initialize the pins

//	GPIOA->MODER &= ~GPIO_MODER_MODE9;
//	GPIOA->MODER |= GPIO_MODER_MODE9_1;	// set PA9 to AF MODE


	GPIOB->MODER &= ~GPIO_MODER_MODE6;
	GPIOB->MODER |=  GPIO_MODER_MODE6_1;

	GPIOB->MODER &= ~GPIO_MODER_MODE7;
	GPIOB->MODER |= GPIO_MODER_MODE7_1;
										// set PB6 & PB7 to be AF MODE
	GPIOC->MODER &= ~GPIO_MODER_MODE6;
	GPIOC->MODER |= GPIO_MODER_MODE6_1;


	GPIOB->MODER &= ~GPIO_MODER_MODE3;
	GPIOB->MODER |= GPIO_MODER_MODE3_0;	// 01
										// set PB3 output-mode
	GPIOB->MODER &= ~GPIO_MODER_MODE4;
	GPIOB->MODER |= GPIO_MODER_MODER4_0;
										// test PB4 output
	GPIOB->MODER &= ~GPIO_MODER_MODE5;
	GPIOB->MODER |= GPIO_MODER_MODE5_0;


	GPIOA->MODER &= ~GPIO_MODER_MODE5;
	GPIOA->MODER |= GPIO_MODER_MODE5_0;

	GPIOB->MODER &= ~GPIO_MODER_MODE9;
	GPIOB->MODER |= GPIO_MODER_MODE9_0;

	GPIOA->MODER &= ~GPIO_MODER_MODE0;
	GPIOA->MODER |= GPIO_MODER_MODE0_1;	// set PA0 to be AF MODE

	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL0;

	//GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL9;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL6;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL7;	// mask them out

	GPIOC->AFR[0] &= ~GPIO_AFRL_AFSEL6;

	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL0_0;	// PA0 AF1

	GPIOB->AFR[0] |=
					 GPIO_AFRL_AFSEL6_0 |
					 GPIO_AFRL_AFSEL6_1 |
					 GPIO_AFRL_AFSEL6_2 |
					 GPIO_AFRL_AFSEL7_0 |
					 GPIO_AFRL_AFSEL7_1 |
					 GPIO_AFRL_AFSEL7_2 ;
										// set PB6 AF7, PB7 AF7;
	GPIOC->AFR[0] |= GPIO_AFRL_AFSEL6_1;
//	GPIOA->AFR[1] |= GPIO_AFRH_AFSEL9_0 |
	//				 GPIO_AFRH_AFSEL9_1 |
		//			 GPIO_AFRH_AFSEL9_2 ;
										// set PA9 AF7;
	// initialize the button

	GPIOC->MODER &= ~GPIO_MODER_MODE13;	// set PC13 input

	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD13;
	GPIOC->PUPDR |= GPIO_PUPDR_PUPD13_0;	// set PC13 pull-up
}


void USART1_init()
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	USART1->CR1 &= ~USART_CR1_M;		// choose world length to be 8(bytes)

	USART1->CR1	 &= ~USART_CR1_OVER8;		// over8 = 0, ovsmpling 16 BRR = USARTDIV

//	USART1->CR1 &= ~USART_CR1_PS;		// We don't need parity selection
	USART1->CR1 &= ~USART_CR1_PCE;		// no parity check

	USART1->CR1 &= ~USART_CR1_TE;
	USART1->CR1 &= ~USART_CR1_RE;
	USART1->CR1	 |= USART_CR1_TE | USART_CR1_RE;
												// reset TE & RE once to refresh the status;
	USART1->CR2 &= ~USART_CR2_STOP;
									// set 1-stop-bit

	USART1->CR3 &= ~USART_CR3_RTSE;
	USART1->CR3 &= ~USART_CR3_CTSE;		// no hardware flowctl
	USART1->CR3 &= ~USART_CR3_ONEBIT;	// choose three-bit sampling ? why?


	USART1->CR2 &= ~(USART_CR2_LINEN | USART_CR2_CLKEN);	// LIN mode disable // clk disable (we use asychronous
	USART1->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);	// SmartCard disabled // We don't use half-duplex // IrDA disabled
//	USART1->CR3 &= ~(USART_CR3_SCEN | USART_CR3_IREN);	// SmartCard disabled // We don't use half-duplex // IrDA disabled
//	USART1->CR3 |= USART_CR3_HDSEL;
// set BRR = USARTDIV
	USART1->BRR = 4000000L / 9200L ;		// I need to set baud rate at 9200 so that 9600 in PC here can catch clear why?
// enable USART
USART1->CR1 |= (USART_CR1_UE);
}
//void unable_Systick()
//{
//SysTick->CTRL &= 0xFFFFFFFC;
//}
int USART1_transmit(uint8_t *arr,uint32_t size){
int k = 0;
for(int i = 0; i < size; i++)
	{
	while(!(USART1->ISR & USART_ISR_TC));
	USART1->TDR = arr[i];
	k++;
	}
while(!(USART1->ISR & USART_ISR_TC));

return k;
}

void itoab(float a)
{
int n = 0;
a *= 10000.0f;
int d = (int)a;
while(d > 0)
	{
	int b = d % 10;
	d /= 10;
	msg[n++] = b + 48;
	if(n == 4)
		{
		msg[n++] = '.';
		}
	}
msg[n] = '\0';
}
void strrev()
{
int a = 0;
int b = strlen(msg) - 1;
while(a <= b)
	{
	char c = msg[a];
	msg[a] = msg[b];
	msg[b] = c;
	a++;
	b--;
	}
}
void adjust()
{
int a = strlen(msg);
msg[a] = '\r';
msg[a + 1] = '\n';
msg[a + 2] = '\0';
}
void refresh()
{
for(int i = 0; i < 32; i++)
	{
	msg[i] = '\0';
	}
}
void transmit_resistor()
{
refresh();
itoab(lux);
strrev();
adjust();
USART1_transmit(msg, strlen(msg));
}
void USART1_RECEIVE(char *c)
{
	state = 0;
	Systick_Config();
	while(!(USART1->ISR & USART_ISR_RXNE));	// while there is nothing to read
	*c = USART1->RDR;
}
/*
void ADC1_start()
{
	ADC1->CR |= ADC_CR_ADSTART;
}
*/

void refresh_TIM3()
{
	uint16_t tmp;
	tmp = TIM3->EGR;
	tmp &= 0xFFFE;
	tmp |= 0x1;		// reinitialize TIM3 & TIM2
	TIM3->EGR = tmp;

	tmp = TIM2->EGR;
	tmp &= 0xFFFE;
	tmp |= 0x1;		// reinitialize TIM3 & TIM2
	TIM2->EGR = tmp;
}
void PWM_channel_init()
{

uint32_t tmp;
uint16_t tmp2;
//TIM4->CCR1 = cycle[now_rate];			// We use floor(99 / 2) = 50
//TIM3->CCR1 = 50;
TIM3->CCR1 = 50;
TIM2->CCR1 = 50;



tmp2 = TIM3->CCER;
tmp2 &= 0xFFFE;	// first mask CC1E to make CC1S writable
TIM3->CCER = tmp2;

tmp2 = TIM2->CCER;
tmp2 &= 0xFFFE;	// first mask CC1E to make CC1S writable
TIM2->CCER = tmp2;




tmp = TIM3->CCMR1;
tmp &= 0xFFFFFFFC;	// mask CC1S to choose to be output
TIM3->CCMR1 = tmp;

tmp = TIM2->CCMR1;
tmp &= 0xFFFFFFFC;	// mask CC1S to choose to be output
TIM2->CCMR1 = tmp;


tmp = TIM3->CCMR1;
tmp &= 0xFFFFFF8F;	// mask OC1M
tmp |= 0x60;		// set OC1M PWM_mode1;
TIM3->CCMR1 = tmp;

tmp = TIM2->CCMR1;
tmp &= 0xFFFFFF8F;	// mask OC1M
tmp |= 0x60;		// set OC1M PWM_mode1;
TIM2->CCMR1 = tmp;

tmp = TIM3->CCMR1;
tmp &= 0xFFFFFFF7;	// mask OC1PE
tmp |= 0x8;		// enable OC1PE
TIM3->CCMR1 = tmp;

tmp = TIM2->CCMR1;
tmp &= 0xFFFFFFF7;	// mask OC1PE
tmp |= 0x8;		// enable OC1PE
TIM2->CCMR1 = tmp;


tmp2 = TIM3->CR1;
tmp2 &= 0xFF7F;
tmp2 |= 0x80;		// enable ARPE;
TIM3->CR1 = tmp2;


tmp2 = TIM2->CR1;
tmp2 &= 0xFF7F;
tmp2 |= 0x80;		// enable ARPE;
TIM2->CR1 = tmp2;



tmp2 = TIM3->CCER;
tmp2 &= 0xFFFC;		// mask CC1P & CC1E
tmp2 |= 0x3;		// configured them both 1, now active-low,OC1 on
TIM3->CCER = tmp2;

tmp2 = TIM2->CCER;
tmp2 &= 0xFFFC;		// mask CC1P & CC1E
tmp2 |= 0x3;		// configured them both 1, now active-low,OC1 on
TIM2->CCER = tmp2;



refresh_TIM3();

}
void timer_init()	//initialize TIM4 here
{

uint32_t tmp;
tmp = RCC->APB1ENR1;
tmp &= 0xFFFFFFFC;
tmp |= 0x3;			// enable TIM3 & TIM2 first
RCC->APB1ENR1 = tmp;


uint16_t tmp2;


tmp2 = TIM3->CR1;
tmp2 &= 0xFF8F;		// mask CMS and DIR to make down_counting and edge aligned
tmp2 |= 0x10;
TIM3->CR1 = tmp2;

tmp2 = TIM2->CR1;
tmp2 &= 0xFF8F;		// mask CMS and DIR to make down_counting and edge aligned
tmp2 |= 0x10;
TIM2->CR1 = tmp2;


//TIM4->PSC = 40000/440-1;
//TIM4->PSC = 40000/261.6 - 1;
//TIM4->PSC = 40000/293.7 - 1;
//TIM4->PSC = 40000/329.6 - 1;
//TIM4->PSC = 40000/349.2 - 1;
//TIM4->PSC = 40000/392-1;
//TIM4->PSC = 40000/440-1;
//TIM4->PSC = 40000/493.9 - 1;
TIM3->PSC = 40000/523.3-1;
TIM2->PSC = 40000/523.3-1;

TIM3->ARR = 99;
TIM2->ARR = 99;

refresh_TIM3();


}
void timer3_start()	// start TIM4 HERE
{
uint16_t tmp;
tmp = TIM3->CR1;
tmp &= 0xFFFE;		// mask CEN
tmp |= 0x1;			// enable CEN
TIM3->CR1 = tmp;	// officially cnting start!
}
void timer3_suspend()
{
uint16_t tmp;
tmp = TIM3->CR1;
tmp &= 0xFFFE;		// mask CEN
TIM3->CR1 = tmp;	// suspend cnting by unable the CEN
}

void timer2_start()	// start TIM4 HERE
{
uint16_t tmp;
tmp = TIM2->CR1;
tmp &= 0xFFFE;		// mask CEN
tmp |= 0x1;			// enable CEN
TIM2->CR1 = tmp;	// officially cnting start!
}
void timer2_suspend()
{
uint16_t tmp;
tmp = TIM2->CR1;
tmp &= 0xFFFE;		// mask CEN
TIM2->CR1 = tmp;	// suspend cnting by unable the CEN
}

void SysTick_Handler()
{
if(state == 0)
	{
	suspend();
	}
}
void Delay(int sec){
	for(int i = 0; i < sec; i++)
			Delay_1s();

}
void adjust_frequency3(int f)
{
	TIM3->PSC = 40000 /  f - 1;
}
void adjust_frequency2(int f)
{
	TIM2->PSC = 40000 /  f - 1;
}
void draw_bet(int sec)
{
right();
Delay(sec);
}
void draw_C()
{
	ul();
	Delay(1);
	up();
	Delay(4);
	ur();
	Delay(1);

	right();
	Delay(2);

	dr();
	Delay(1);
	ul();
	Delay(1);

	left();
	Delay(2);

	dl();
	Delay(1);
	down();
	Delay(4);
	dr();
	Delay(1);

	right();
	Delay(2);

	ur();
	Delay(1);
	dl();
	Delay(1);

}
void draw_N()
{
	down();
	Delay(6);
	up();
	Delay(6);
	adjust_frequency3(600);
	dr();
	Delay(6);
	adjust_frequency3(800);
	up();
	Delay(6);
	down();
	Delay(6);
}
void draw_T()
{
	up();
	Delay(6);

	left();
	Delay(2);

	right();
	Delay(4);

	left();
	Delay(2);

	down();
	Delay(6);
}
void draw_U()
{
	left();
	Delay(2);

	ul();
	Delay(1);
	up();
	Delay(5);

	down();
	Delay(5);
	dr();
	Delay(1);

	right();
	Delay(2);

	ur();
	Delay(1);
	up();
	Delay(5);

}
void draw_NCTU()
{
draw_N();
draw_bet(3);
draw_C();
draw_bet(3);
draw_T();
draw_bet(6);
draw_U();
suspend();


}
void draw_big_NCTU()
{
down();
Delay(6);

right();
Delay(1);

up();
Delay(5);

adjust_frequency3(600);
dr();
Delay(5);
adjust_frequency3(800);

right();
Delay(18);

up();
Delay(1);

left();
Delay(1);

ur();
Delay(1);

up();
Delay(4);

left();
Delay(1);

down();
Delay(3);

dl();
Delay(1);

left();
Delay(1);

ul();
Delay(1);

up();
Delay(3);

left();
Delay(1);

down();
Delay(4);

dr();
Delay(1);

left();
Delay(4);

up();
Delay(4);

right();
Delay(2);

up();
Delay(1);

left();
Delay(5);

down();
Delay(1);

right();
Delay(2);

down();
Delay(4);

left();
Delay(3);

up();
Delay(1);

left();
Delay(2);

ul();
Delay(1);

up();
Delay(1);

ur();
Delay(1);

right();
Delay(2);

up();
Delay(1);

left();
Delay(3);

dl();
Delay(1);

down();
Delay(3);

dr();
Delay(1);

left();
Delay(2);

up();
Delay(5);

left();
Delay(1);

down();
Delay(5);

adjust_frequency3(600);
ul();
Delay(5);
adjust_frequency3(800);

left();
Delay(1);

suspend();
}
void suspend()
{
	timer2_suspend();
	timer3_suspend();
}
/*
void ADC1_2_IRQHandler()
{
	while(!(ADC1->ISR & ADC_ISR_EOC));	// ensure that coversion complete
	float a;
	float ADC_value = 5.0f / 1024.0f;		// now resolution12-bit 4096,we use 5.0v
//	int LDR_value = ADC1->DR;		// LDR = Light detect resistor_value is read from ADC1->DR.
	float LDR_value = (float)ADC1->DR;
//	float LDR_value = ADC1->DR;
	int b = ADC1->DR;
	lux = (float)ADC1->DR;
//	lux = (250 / (5 * LDR_value / 4096)) - 50;
}

*/
void up()
{

					enable();
					//disable_two();
					third_reclock();
					timer2_start();
					timer3_suspend();
}
void down()
{
					enable();
					//disable_two();
					third_clock();
					timer2_start();
					timer3_suspend();

}
void left()
{

					enable();
					//disable_third();
					two_reclock();
					timer3_start();
					timer2_suspend();
}
void right()
{

					enable();
					//disable_third();
					two_clock();
					timer3_start();
					timer2_suspend();

}
void ul()
{
		enable();
		third_reclock();
		timer2_start();
		two_reclock();
		timer3_start();
}
void ur()
{
		enable();
		third_reclock();
		timer2_start();
		two_clock();
		timer3_start();
}
void dl()
{
		enable();
		third_clock();
		timer2_start();
		two_reclock();
		timer3_start();
}
void dr()
{
			enable();
			third_clock();
			timer2_start();
			two_clock();
			timer3_start();
}
void draw_triangle()
{
	dl();
	Delay(3);
	right();
	Delay(6);
	ul();
	Delay(3);
	suspend();
}
void read_from_input(){
char c1;
	now = 0;
	do
	{
	USART1_RECEIVE(&c1);
	state = 1;
		if(c1 == '\r')	// if we meet backcar then there must be an enter
			{
			USART1_transmit((uint8_t *)"\r\n",2);
			c1 = '\0';		// we have read a line successfully
			}
		else if(c1 == '\b')//'\x7F')	// if we want to delete a char
			{
			if(now > 0)
				{
				cmd[now--] = '\0';
				}
			USART1_transmit("\b\x20\b",3);
			}
		else				// all other cases, count this char into account
			{
			if(c1 == 'd' || c1 == 'D')
			//if(c1 == '1')
				{
				adjust_frequency3(800);
				right();
				}
			else if(c1 == 'a' || c1 == 'A')
			//else if(c1 == '2')
				{
				adjust_frequency3(800);
				left();
				}
			else if(c1 == 's' || c1 == 'S')
		//	else if(c1 == '3')
				{
				adjust_frequency2(800);
				down();
				}
			else if(c1 == 'W' || c1 == 'w')
				{
				adjust_frequency2(800);
				up();
				}
			else if(c1 == 'Q' || c1 == 'q')
							{
							adjust_frequency2(800);
							adjust_frequency3(800);
							//adjust_frequency3(600);
							ul();
							}
			else if(c1 == 'E' || c1 == 'e')
							{
							adjust_frequency2(800);
							adjust_frequency3(800);
							//adjust_frequency3(600);
							ur();
							}
			else if(c1 == 'Z' || c1 == 'z')
							{
							adjust_frequency2(800);
							adjust_frequency3(800);
							//adjust_frequency3(600);
							dl();
							}
			else if(c1 == 'C' || c1 == 'c')
							{
							adjust_frequency2(800);
							adjust_frequency3(800);
							//adjust_frequency3(600);
							dr();
							}
			else if(c1 == 'N' || c1 == 'n')
							{
							adjust_frequency2(800);
							adjust_frequency3(800);
							draw_NCTU();
							}
			else if(c1 == 'T' || c1 == 't')
							{
							adjust_frequency2(800);
							adjust_frequency3(600);
							draw_triangle();
							}
			else if(c1 == 'Y' || c1 == 'y')
							{
							adjust_frequency2(800);
							adjust_frequency3(800);
							draw_big_NCTU();
							}
			else
				{
				suspend();
				}
			cmd[now++] = c1;
			USART1_transmit(&c1,1);
			}
		}while(c1 != '\0');
cmd[now] = '\0';
}
void two_clock()	// set PB3 0 for active low;
{
GPIOB->ODR &= 0xFFFFFFF7;
}
void two_reclock()		// set PB3 1
{
GPIOB->ODR &= 0xFFFFFFF7;
GPIOB->ODR |= 0x8;
}
void third_clock()
{
GPIOA->ODR &= 0xFFFFFFDF;
}
void third_reclock()
{
GPIOA->ODR &= 0xFFFFFFDF;
GPIOA->ODR |= 0x20;
}
void enable_two()
{
	GPIOB->ODR &= 0xFFFFFFDF;
}
void disable_two()
{
	GPIOB->ODR &= 0xFFFFFFDF;
	GPIOB->ODR |= 0x20;
}
void disable_third()
{
	GPIOB->ODR &= 0xFFFFFFEF;
	GPIOB->ODR |= 0x10;
}
void enable_third()
{
	GPIOB->ODR &= 0xFFFFFFEF;
}
void enable()
{
	enable_two();
	enable_third();
}
int check_q()
{
while(USART1->ISR & USART_ISR_RXNE)
	{
	char c1;
	c1 = USART1->RDR;
	USART1_transmit((uint8_t *)&c1,1);
	if(c1 == 'q')
			return 1;
	}
return 0;
}
void str_cpy(char *c1,char *c2)
{
int cnt = 0;
for(int i = 0; i < strlen(c2); i++)
	{
	c1[cnt++] = c2[i];
	}
c1[cnt] = '\0';
}
int str_cmp(char *c1,char *c2)
{
if(strlen(c1) != strlen(c2))
	{
	return -1;
	}
else
	{
	for(int i = 0; i < strlen(c1); i++)
		{
		if(c1[i] != c2[i])
			return -1;
		}
	return 0;
	}
}
void led_show()
{
uint32_t tmp = 0x8;
tmp &= GPIOB->ODR;
if(tmp == 0)
	{
	USART1_transmit((uint8_t *)"LIGHT\r\n",7);
	}
else
	{
	USART1_transmit((uint8_t *)"DARK\r\n",7);
	}
}
void main()
{
unsigned char err[32] = "[Invalid Command]\r\n";
fpu_enable();
GPIO_init();
//ADC1_init();
timer_init();
PWM_channel_init();
Systick_Config();
int a;

/*
Delay_1s();
Delay_1s();
Delay_1s();
Delay_1s();
Delay_1s();
Delay_1s();
Delay_1s();
Delay_1s();
Delay_1s();
*/

do {
a = Delay_1s();
}while(a != 1);
USART1_init();			// after we ensure to open then open
						// or noise will destroy the program

while(1){

		str_cpy(msg,"> ");
		USART1_transmit(msg, strlen(msg));
	read_from_input();

	if(!str_cmp(cmd,"showid"))
				{
				}



		str_cpy(msg,"> ");
		USART1_transmit(msg, strlen(msg));
		//Delay_1s();
		read_from_input();
		if(!str_cmp(cmd,"showid"))
			{
			}
		else if(!str_cmp(cmd,"led on"))
			{

			}
		else if(!str_cmp(cmd,"led off"))
			{

			}
		else if(!str_cmp(cmd,"led show"))
			{
			}
		else if(!str_cmp(cmd,"light"))
			{
		//	int q = 0;
			//do
				//{
			//	transmit_resistor();
				//Delay_1s();
				//q = check_q();
				//}while(!q);
			}
		else if(strlen(cmd) == 0)
			{
			continue;
			}
		else
			{
			//int a = strlen(cmd) > 0?strlen(cmd) : 0;
			//itoab(now);
			//USART1_transmit(msg,strlen(msg));
			//USART1_transmit((uint8_t *)"Invalid Command\r\n",17);
			}

}

}


