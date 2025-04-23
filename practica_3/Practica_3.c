
#include <MKL25Z4.H>
#include <stdio.h>
#include <stdlib.h>

#define RS 0x04 /* PTA2 mask */
#define RW 0x10 /* PTA4 mask */
#define EN 0x20 /* PTA5 mask */

void delayMs(int n);

void LCD_command(unsigned char command);

void LCD_command_noWait(unsigned char command);

void LCD_data(unsigned char data);

void LCD_init(void);

void keypad_init(void);

char keypad_getkey(void);

int decoder_teclado(char input);
int getTimer(void);

void LED_init(void);

int main(void)
{
	char str[4];
	int timerNum=0;

	/* Init code */
	SIM->SCGC6 |= 0x01000000; /* enable clock to TPM0 */
	SIM->SOPT2 |= 0x01000000; /* use 32.76khz as clock */
	TPM0->SC = 0; /* disable timer while configuring */
	TPM0->SC = 0x02; /* prescaler /4 */
	TPM0->MOD = 0x2000; /* max modulo value 8192*/
	TPM0->SC |= 0x80; /* clear TOF */
	TPM0->SC |= 0x08; /* enable timer free-running mode */

	LCD_init();
	keypad_init();

	while(1){
	timerNum = 0;
		LCD_command(1);    /* clear display */
		delayMs(500);
		LCD_command(0xC0); /* set cursor at 2nd line */
		LCD_data('h');     /* write the word on LCD */
		LCD_data('e');
		LCD_data('l');
		LCD_data('l');
		LCD_data('o');
		delayMs(5000); //Wait 5 seconds
		LCD_command(0x80); /* set cursor at 1st line */
		LCD_data('I');     /* write the word on LCD */
		LCD_data('n');
		LCD_data('t');
		LCD_data('r');
		LCD_data('o');
		LCD_data('d');
		LCD_data(':');
		LCD_data(' ');

		//LLAMAR EL LOOP DE GETKEY() QUE TIENE QUE RECIBIR DOS INPUTS Y CONVERTIRLOS EN UN INT
		timerNum = getTimer();
		PTD->PSOR = 0x02;
		LCD_command(1);    /* clear display */
		LCD_command(0x80);
		//Imprimir el timer
		sprintf(str, "%d", timerNum);
		for (int i = 0; str[i] != '\0'; i++) {
			LCD_data(str[i]);
		}

		//INICIAR EL TIMER!!!
		delayMs(timerNum*1000);

		//Cuando acaba, hacer un toggle de un led y reiniciar el loop
		/* turn on red LED */
		PTB->PCOR = 0x40000;
		delayMs(500);
		PTB->PSOR = 0x40000;
		delayMs(500);
	}
	}

int getTimer(void){
	while(1){
	//LLAMAR EL LOOP DE GETKEY() QUE TIENE QUE RECIBIR DOS INPUTS Y CONVERTIRLOS EN UN INT
				int timerNum0 = 0;
				int num1 = -3;
				int num2 = -3;
				int num3 = -3;

				//Repetir getKey hasta que haya un input
				while(num1 == -3){
				num1 = decoder_teclado(keypad_getkey());}
				timerNum0 += num1*10;
				delayMs(1000);

				//Repetir getKey hasta que haya un input
				while(num2 == -3){
				num2 = decoder_teclado(keypad_getkey());}
				timerNum0 += num2;
				delayMs(1000);

				//Repetir getKey hasta que haya un input
				while(num3 == -3){
				num3 = decoder_teclado(keypad_getkey());}
				//Checar que sí haya sido *
				if(num3 == -1){return timerNum0;}
				delayMs(1000);
}}

void keypad_init(void)
{
SIM->SCGC5 |= 0x0800; /* enable clock to Port C */
PORTC->PCR[0] = 0x103; /* PTD0, GPIO, enable pullup*/
PORTC->PCR[1] = 0x103; /* PTD1, GPIO, enable pullup*/
PORTC->PCR[2] = 0x103; /* PTD2, GPIO, enable pullup*/
PORTC->PCR[3] = 0x103; /* PTD3, GPIO, enable pullup*/
PORTC->PCR[4] = 0x103; /* PTD4, GPIO, enable pullup*/
PORTC->PCR[5] = 0x103; /* PTD5, GPIO, enable pullup*/
PORTC->PCR[6] = 0x103; /* PTD6, GPIO, enable pullup*/
PORTC->PCR[7] = 0x103; /* PTD7, GPIO, enable pullup*/
PTC->PDDR = 0x0F; /* make PTD7-0 as input pins */
}

void LCD_init(void){
	SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
	PORTD->PCR[0] = 0x100;/* make PTD0 pin as GPIO */
	PORTD->PCR[1] = 0x100;/* make PTD1 pin as GPIO */
	PORTD->PCR[2] = 0x100;
	PORTD->PCR[3] = 0x100;
	PORTD->PCR[4] = 0x100;
	PORTD->PCR[5] = 0x100;
	PORTD->PCR[6] = 0x100;
	PORTD->PCR[7] = 0x100;/* make PTD7 pin as GPIO */
	PTD->PDDR = 0xFF;  /* make PTD7-0 as output pins */
	SIM->SCGC5 |= 0x0200; /* enable clock to Port A */
	PORTA->PCR[2] = 0x100;/* make PTA2 pin as GPIO */
	PORTA->PCR[4] = 0x100;/* make PTA4 pin as GPIO */
	PORTA->PCR[5] = 0x100;/* make PTA5 pin as GPIO */
	PTA->PDDR |= 0x34; /*make PTA5, 4, 2 as output pins*/

	delayMs(20); /* initialization sequence */
	/* LCD does not respond to status poll */
	LCD_command_noWait(0x30);
	delayMs(5);
	LCD_command_noWait(0x30);
	delayMs(1);
	LCD_command_noWait(0x30);
	/* set 8-bit data, 2-line, 5x7 font */
	LCD_command(0x38);
	/* move cursor right */
	LCD_command(0x06);
	/* clear screen, move cursor to home */
	LCD_command(0x01);
	/* turn on display, cursor blinking */
	LCD_command(0x0F);}

/* This function waits until LCD controller is ready to accept a new command/data before returns. */
void LCD_ready(void)
{
	char status;
	PTD->PDDR = 0; /* PortD input */
	PTA->PCOR = RS; /* RS = 0 for status */
	PTA->PSOR = RW; /* R/W = 1, LCD output */
	do { /* stay in the loop until it is not busy */
		PTA->PSOR = EN; /* raise E */
		delayMs(0);
		status = PTD->PDIR; /* read status register */
		PTA->PCOR = EN;
		delayMs(0); /* clear E */
	} while (status & 0x80); /* check busy bit */


	PTA->PCOR = RW; /* R/W = 0, LCD input */
	PTD->PDDR = 0xFF; /* PortD output */
}

void LCD_command(unsigned char command)
{
	LCD_ready(); /* wait until LCD is ready */
	PTA->PCOR = RS | RW; /* RS = 0, R/W = 0 */
	PTD->PDOR = command;
	PTA->PSOR = EN; /* pulse E */
	delayMs(0);
	PTA->PCOR = EN;
}

void LCD_command_noWait(unsigned char command){
	PTA->PCOR = RS | RW; /* RS = 0, R/W = 0 */
	PTD->PDOR = command;
	PTA->PSOR = EN; /* pulse E */
	delayMs(0);
	PTA->PCOR = EN; }

void LCD_data(unsigned char data)
{
	LCD_ready(); /* wait until LCD is ready */
	PTA->PSOR = RS; /* RS = 1, R/W = 0 */
	PTA->PCOR = RW;
	PTD->PDOR = data;
	PTA->PSOR = EN; /* pulse E */
	delayMs(0);
	PTA->PCOR = EN;
}

char keypad_getkey(void) {
int row, col;
const char row_select[] = {0x01, 0x02, 0x04, 0x08};
/* one row is active */
/* check to see any key pressed */
PTC->PDDR |= 0x0F; /* enable all rows */
PTC->PCOR = 0x0F;
delayMs(2); /* wait for signal return */
col = PTC-> PDIR & 0xF0; /* read all columns */
PTC->PDDR = 0; /* disable all rows */
if (col == 0xF0)
return 0; /* no key pressed */

/* If a key is pressed, we need find out which key.*/
for (row = 0; row < 4; row++)
{ PTC->PDDR = 0; /* disable all rows */
PTC->PDDR |= row_select[row]; /* enable one row */
PTC->PCOR = row_select[row]; /* drive active row low*/
delayMs(2); /* wait for signal to settle */
col = PTC->PDIR & 0xF0; /* read all columns */
if (col != 0xF0) break;
/* if one of the input is low, some key is pressed. */
}

PTC->PDDR = 0; /* disable all rows */
if (row == 4)
return 0; /* if we get here, no key is pressed */
/* gets here when one of the rows has key pressed*/
/*check which column it is*/
if (col == 0xE0) return row*4+ 1; /* key in column 0 */
if (col == 0xD0) return row*4+ 2; /* key in column 1 */
if (col == 0xB0) return row*4+ 3; /* key in column 2 */
if (col == 0x70) return row*4+ 4; /* key in column 3 */
return 0; /* just to be safe */
}

int decoder_teclado(char input){
	if(input == 0){
		return -3;
	}
	else if(input == 1){
		LCD_data('1');
		return 1;
	}
	else if(input == 2){
		LCD_data('2');
			return 2;
		}
	else if(input == 3){
		LCD_data('3');
				return 3;
			}
	else if(input == 4){
		LCD_data('A');
					return 10;
				}
	else if(input == 5){
		LCD_data('4');
						return 4;
					}
	else if(input == 6){
		LCD_data('5');
							return 5;
						}
	else if(input == 7){
		LCD_data('6');
							return 6;
						}
	else if(input == 8){
		LCD_data('B');
							return 11;
						}
	else if(input == 9){
		LCD_data('7');
							return 7;
						}
	else if(input == 10){
		LCD_data('8');
							return 8;
						}
	else if(input == 11){
		LCD_data('9');
							return 9;
						}
	else if(input == 12){
		LCD_data('C');
							return 12;
						}
	else if(input == 13){
		LCD_data('*');
							return -1;
						}
	else if(input == 14){
		LCD_data('0');
							return 0;
						}
	else if(input == 15){
		LCD_data('#');
							return -2;
						}
	else if(input == 16){
		LCD_data('D');
							return 13;
						}
	else{
		return -3;
	}
}

void LED_init(void)
{
	SIM->SCGC5 |= 0x400; /* enable clock to Port B */
	SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
	PORTB->PCR[18] = 0x100; /* make PTB18 pin as GPIO */
	PTB->PDDR |= 0x40000; /* make PTB18 as output pin */
	PTB->PSOR |= 0x40000; /* turn off red LED */
	PORTB->PCR[19] = 0x100; /* make PTB19 pin as GPIO */
	PTB->PDDR |= 0x80000; /* make PTB19 as output pin */
	PTB->PSOR |= 0x80000; /* turn off green LED */
	PORTD->PCR[1] = 0x100; /* make PTD1 pin as GPIO */
	PTD->PDDR |= 0x02; /* make PTD1 as output pin */
	PTD->PSOR |= 0x02; /* turn off blue LED */
}




/* Delay function */
void delayMs(int n) {
	while((TPM0->SC & 0x80) == 0) { }
	int i;
	SysTick->LOAD = 41940 - 1;
	SysTick->CTRL = 0x5; /* Enable the timer and
	choose sysclk as the clock
	source */
	for(i = 0; i < n/2; i++) {
	while((SysTick->CTRL & 0x10000) == 0)
	/* wait until the COUNT flag is set */
	{ }
	}
	SysTick->CTRL = 0;
	/* Stop the timer (Enable = 0) */
	}
