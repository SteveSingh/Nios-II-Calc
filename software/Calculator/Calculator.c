#include <stdio.h>
#include <io.h>
#include <alt_types.h>
#include <math.h>
#include "alt_up_ps2_port.h"
#include "ps2_keyboard.h"
#include "altera_avalon_lcd_16207_regs.h"
#include "alt_up_character_lcd.h"

unsigned float* Operator1;	//First operator
unsigned float* Operator2;	//Second operator
unsigned float* Memory;		//Used to store an operator
byte*    Op;				//Operation to perform
unsigned float* Result;		//Result of the calculation


int main()
{
	clear_FIFO();			//Clear FIFO of the PS/2 port
	DECODE_MODE decode_mode;
	PS2_DEVICE mode = get_mode(); //Check if mouse or keyboard
	alt_u8 Operater1, Operator2;

	while( mode == PS2_KEYBOARD)
	{
		if (*Op == 0) //Addition
		{
			*Result = (*Operator1) + (*Operator2);
			printf("Result: %d\n", *Result);
		}
		else if (*Op == 1) //Subtraction
		{
			*Result = (*Operator1) - (*Operator2);
			printf("Result: %d\n", *Result);
		}
		else if (*Op == 2) //Multiplication
		{
			*Result = (*Operator1) * (*Operator2);
			printf("Result: %d\n", *Result);
		}
		else if (*Op == 3) //Division
		{
			*Result = (*Operator1) / (*Operator2);
			printf("Result: %d\n", *Result);
		}
		else if (*Op == 4) //Memory store
		{
			*Memory = *Operator1;
			printf("\nCurrent Memory value: %d\n", *Result);
		}
		else if (*Op == 5) //Memory clear
		{
			*Memory = 0;
			printf("\nCurrent Memory value: %d\n", *Result);
		}
		else if (*Op == 6) //Sine
		{
			*Result = sin((*Operator1));
			printf("Result: %d\n", *Result);
		}
		else if (*Op == 7) //Cosine
		{
			*Result = cos((*Operator1));
			printf("Result: %d", *Result);
		}
		else if (*Op == 8) //Tangent
		{
			*Result = tan((*Operator1));
			printf("Result: %d", *Result);
		}
		else if (*Op == 9) //Logarithm
		{
			*Result = log10((*Operator1));
			printf("Result: %d", *Result);
		}
		else if (*Op == 10) //Tangent
		{
			*Result = pow((*Operator1),(*Operator2));
			printf("Result: %d", *Result);
		}
		else
		{
			printf("Waiting for an operation...\n");
		}
	}
}
