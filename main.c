/* File Comment
 * ---------------------------------------------------------------------------------------------------
 * [FILE NAME]: <filename.c here>
 *
 * [AUTHOR]: Abdallah Hossam-Eldin Hosny
 *
 * [DATE CREATED]: <put date>
 *
 * [DESCRIPTION]: <decsription here>
 * ---------------------------------------------------------------------------------------------------
 */

#include "micro_config.h"
#include "common_macros.h"
#include "std_types.h"
#include "lcd.h"
#include "adc.h"

#define ENABLE_INT()		SREG  |= (1<<7);

volatile uint8 g_OnOffFlag = 0;
volatile uint8 g_direction = 0;


void Timer1_init(void);
void INT0_init(void);
void INT1_init(void);


ISR(INT0_vect){
	/* To avoid bouncing */
	_delay_ms(15);
	while(BIT_IS_CLEAR(PORTD, PD2));
	_delay_ms(15);
	g_OnOffFlag ^= 1;
}

ISR(INT1_vect){
	/* To avoid bouncing */
	_delay_ms(15);
	while(BIT_IS_CLEAR(PORTD, PD3));
	_delay_ms(15);
	g_direction ^= 1;
}

ISR(TIMER1_COMPA_vect){
	TOGGLE_BIT(PORTC, PC3);
}

int main(void){

	ENABLE_INT();	// Enable global interrupts.
	LCD_init();
	ADC_init();

	INT0_init();
	INT1_init();

	Timer1_init();

	SET_BIT(DDRC, PC0);		// Make PC0 pin as output to select the motor direction.
	SET_BIT(DDRC, PC3);		// Make PC0 pin as output to specify the motor speed.

	// Initially output of both pins is Low
	CLEAR_BIT(PORTC, PC0);
	CLEAR_BIT(PORTC, PC3);

	// Print the main word on LCD
	LCD_displayString((uint8*)"Time = ");
	LCD_displayStringRowColumn(1, 0, (uint8*)"Dir: ");

	uint16 analogVoltage = 0;
	uint16 displayedNumber = 0;

	uint8 dir = 2;

	uint8 OnOff = 2;

	LCD_displayStringRowColumn(0, 13, (uint8*)"O");

	for(;;){
		// Check if the machine is on or off.
		if(OnOff != g_OnOffFlag && g_OnOffFlag){
			SET_BIT(TIMSK, OCIE1A);
			LCD_displayStringRowColumn(0, 14, (uint8*)"N ");
			OnOff = g_OnOffFlag;
		}
		else if(OnOff != g_OnOffFlag && !g_OnOffFlag){
			CLEAR_BIT(TIMSK, OCIE1A);
			LCD_displayStringRowColumn(0, 14, (uint8*)"FF");
			OnOff = g_OnOffFlag;
		}

		// Check if the direction changed by the user or no.
		if(dir != g_direction){
			if (g_direction) {
				LCD_displayStringRowColumn(1, 5, (uint8*)"Reverse");
				CLEAR_BIT(PORTC, PC0);

			} else {
				LCD_displayStringRowColumn(1, 5, (uint8*)"Forward");
				SET_BIT(PORTC, PC0);
			}
			dir = g_direction;
		}
		else{	}

		analogVoltage = ADC_readChannel(0);
		analogVoltage /= 12;
		analogVoltage *= 12;
		analogVoltage = analogVoltage / 9;
		analogVoltage += 7;
		//if(analogVoltage != displayedNumber && (analogVoltage > displayedNumber + 1 || analogVoltage < displayedNumber - 1)){
		if(analogVoltage != displayedNumber){
			OCR1A = (uint16)(analogVoltage * 6 + 1);
			TCNT1 = 0;

			LCD_displayStringRowColumn(0, 7, (uint8*)"    ");
			LCD_goToRowColumn(0, 7);
			LCD_integerToString(analogVoltage);

			displayedNumber = analogVoltage;
		}
		else {	}
	}
}



/* Function Comment
 * ---------------------------------------------------------------------------------------------------
 * [FUNCTION NAME]:			INT0_init
 *
 * [DESCRIPTION]:			Function to initialize INT0 to work with the raising edge
 *
 * [Args]:					None.
 *
 * [IN]:					None.
 *
 * [OUT]:					None.
 *
 * [IN / OUT]:				None.
 *
 * [RETURNS]:				Void.
 * ---------------------------------------------------------------------------------------------------
 */
void INT0_init(void){
	SREG  &= ~(1<<7);                   // Disable interrupts by clearing I-bit
	DDRD  &= (~(1<<PD2));               // Configure INT0/PD2 as input pin
	PORTD |= 1 << PD2;
	GICR  |= (1<<INT0);                 // Enable external interrupt pin INT0
	MCUCR |= (1<<ISC00) | (1<<ISC01);   // Trigger INT0 with the raising edge
	SREG  |= (1<<7);                    // Enable interrupts by setting I-bit
}


/* Function Comment
 * ---------------------------------------------------------------------------------------------------
 * [FUNCTION NAME]:			INT1_init
 *
 * [DESCRIPTION]:			Function to initialize INT1 to work with the raising edge
 *
 * [Args]:					None.
 *
 * [IN]:					None.
 *
 * [OUT]:					None.
 *
 * [IN / OUT]:				None.
 *
 * [RETURNS]:				Void.
 * ---------------------------------------------------------------------------------------------------
 */
void INT1_init(void){
	SREG  &= ~(1<<7);                   // Disable interrupts by clearing I-bit
	DDRD  &= (~(1<<PD3));               // Configure INT0/PD2 as input pin
	PORTD |= (1 << PD3);
	GICR  |= (1<<INT1);                 // Enable external interrupt pin INT0
	MCUCR |= (1<<ISC10) | (1<<ISC11);   // Trigger INT0 with the raising edge
	SREG  |= (1<<7);                    // Enable interrupts by setting I-bit
}


/* Function Comment
 * ---------------------------------------------------------------------------------------------------
 * [FUNCTION NAME]:			Timer1_init
 *
 * [DESCRIPTION]:			Function to initialize Timer1 to work in CTC mode with prescalre 256
 *
 * [Args]:					None.
 *
 * [IN]:					None.
 *
 * [OUT]:					None.
 *
 * [IN / OUT]:				None.
 *
 * [RETURNS]:				Void.
 * ---------------------------------------------------------------------------------------------------
 */
void Timer1_init(void){
	SREG  &= ~(1<<7);                   // Disable interrupts by clearing I-bit
	SET_BIT(TCCR1A, FOC1A);
	SET_BIT(TCCR1A, FOC1B);
	SET_BIT(TCCR1B, WGM12);
	SET_BIT(TCCR1B, CS12);
	TCNT1 = 0;
	OCR1A = 65000;
	SREG  |= (1<<7);                    // Enable interrupts by setting I-bit
}
