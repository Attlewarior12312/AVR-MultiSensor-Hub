/*
 * Projekt.c
 *
 * Created: 22.12.2023 20:14:42
 * Author : PC
 */ 

#define F_CPU 16000000UL								// Przyk³adowa wartoœæ dla czêstotliwoœci CPU
#include <avr/io.h>										/* Include AVR std. library file */
#include <util/delay.h>									/* Include delay header file */
#include <inttypes.h>									/* Include integer type header file */
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/sfr_defs.h>								/* Include standard library file */
#include <stdio.h>										/* Include standard library file */
#include "lcd_displ.h"
#include <math.h>
#include <avr/pgmspace.h>

#define REF_11 (1<<REFS1)|(1<<REFS0)

#define A_COEFFICIENT 1.009249522e-03
#define B_COEFFICIENT 2.378405444e-04
#define C_COEFFICIENT 2.019202697e-07

#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define R0 5000
#define B 3950
#define T0 298.15 // 25 degrees Celsius in Kelvin

// Funkcja przeliczaj¹ca wartoœæ termistora na stopnie Celsius
double ADC_to_Temperature(uint16_t adcValue)
{
	
    uint16_t adc_value = adcValue;
    double R = R0 * (1023.0 / adc_value - 1.0);

    // Przekszta³cenie oporu na temperaturê w kelwinach
    double temp_kelvin = 1.0 / (1.0 / T0 + log(R / R0) / B);

    // Konwersja temperatury na stopnie Celsiusza
    //double temp_celsius = abs(temp_kelvin - 273.15);
	double temp_celsius = temp_kelvin - 273.15;

    return (uint16_t)temp_celsius;
}

double steinhart_hart(unsigned int resistence)
{
	double temperature;

	/*
	Temperature (K) = 1 / (a + (b*ln(Rntc) + c.ln(Rntc)*ln(Rntc)*ln(Rntc)))

	a = 0.0011303
	b = 0.0002339
	c = 0.00000008863
	*/
	temperature = log(resistence);
	temperature = 1 / (0.0011303 + (0.0002339 * temperature) + (0.00000008863 * temperature * temperature * temperature));
	
	//Convert in Celsius
	temperature = temperature - 273.15;

	return temperature;
}



uint16_t pomiar (uint8_t kanal){
	//ADCSRA |= _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);
	ADMUX = (ADMUX & 0b1111100)|kanal; //maskowanie 3 osatmoch bitów
	ADCSRA |= (1<<ADSC);// start konwersji
	
	while(ADCSRA & (1<<ADSC));
	return ADCW; //zwrot pary rejestrów jako zmienna 16-bitowa (makro zaimplemeowane w GCC)
}

uint16_t odczytADC(uint8_t pin){
	//Clear Pins from MUX bits
	ADMUX &= 0xF8;

	ADMUX |= pin;
	ADCSRA |= (1 << ADSC);
	
	while ((ADCSRA & (1 << ADIF)) == 0) _delay_ms(1);

	ADCSRA |= (1 << ADIF);

	return ADC;
}

void USART_Init( uint16_t ubrr)
{
/*Set baud rate */
UBRR0H = (uint8_t)(ubrr>>8);
UBRR0L = (uint8_t)ubrr;
/*Enable receiver and transmitter */
UCSR0B = (1<<TXEN0);
/* Set frame format: 8data, 2stop bit */
//UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

void USART_put(unsigned char *s){
	while( *s ) USART_Transmit(*s++);
}

void print_to_USART(const char *format, ...) {
	va_list args;
	va_start(args, format);

	// ZnajdŸ d³ugoœæ sformatowanego stringa
	int len = vsnprintf(NULL, 0, format, args);
	va_end(args);

	// Zarezerwuj pamiêæ dla sformatowanego stringa
	char buffer[len + 1];
	va_start(args, format);
	vsnprintf(buffer, len + 1, format, args);
	va_end(args);

	// Wywo³aj funkcjê USART_put, aby wys³aæ sformatowany string
	USART_put(buffer);
}

void USART_put_P(const char *s,...){
	register char c;
	while ((c = pgm_read_byte(s++)));
}

void USART_putint(uint16_t liczba,uint8_t radix,...){
	char buf[17];
	itoa(liczba,buf,radix); 
	USART_put(buf);
}

void timer1_conf() {
	DDRB |= (1<<PB1); // ustwaienie PB1 jako wyjœcia
	
	TCCR1A = (1<<WGM11) | (1<<COM1A1) ; //Fast-PWM / 64 prescaler
	TCCR1B = (1<<CS10)| (1<<CS11) | (1<<WGM12) | (1<<WGM13);
		
	ICR1 = 4999; // 50 = 16Mhz / (64* (1 + T) --> T = 4999 | 10 = 16Mhz / (64* (1 + T) --> T = 24999
	
	
	/*TCCR1A |= (1<<COM1A1) | (1<<WGM11); // Szybki pwm, brak poprawnosci fazowej, ustawienie OC1A na dole fazy
	TCCR1B |= (1<<WGM13) | (1<<WGM12) | (1<<CS11); // preskaler 8
	ICR1=39999;   //20ms sygnal PWM  OCR1A = 3999 -180 deg, ICR1A = 1999 - 0deg. Wylicza do wartosci 39999, dobrano na podstawie ICR1 = taktowanie procesora/preskaler zegara/pozadana czastotliwosc w Hz
	*/
}

// Funkcja inicjalizuj¹ca Timer2
void timer2_conf() {
	// Ustawienie preskalera na 64 (okres jednego licznika = 64 / F_CPU)
	TCCR2B |= (1 << CS22) | (1 << CS21);

	// Ustawienie licznika
	TCNT2 = 0;
	
	// W³¹czenie przerwañ od przepe³nienia
	TIMSK2 |= (1 << TOIE2);
	
	// W³¹czenie globalnych przerwañ
	sei();
}



void timer0_conf() {
	//tabelki 162
	TCCR0A = (1<<WGM01);//Ustawienie CTC i ustawienie preskalera na 1024
	TCCR0B =(1<<CS02)|(1<<CS00);
	TIMSK0 = (1<<OCR0A);//w³¹czenie przewania
	OCR0A=249;
	DDRB |= (1 << PB3);  // Ustaw pin PB3 (OC2A) jako wyjœcie

	// set up timer with prescaler = 256 and CTC mode
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS02);

	// initialize counter
	TCNT0 = 0;

	// initialize compare value
	OCR0A = 195;  // Adjust this value based on your requirements

	// enable compare interrupt
	TIMSK0 |= (1 << OCIE0A);

	// enable global interrupts
	sei();
	
}

uint16_t ms; // millisecond counter
uint8_t sec; // second counter
uint8_t min; // minute counter
uint8_t hr; // hour counter

uint16_t ms_button; // duration press of a button
uint8_t state_button = 1; // timer state: 0-reset; 1-paused; 2-continues to count

#define BUTTON PC0 // button switch connected to port C pin 0

#define DEBOUNCE_TIME 25 // time to wait while "de-bouncing" button

#define RESET_BUTTON_DURATION 1000 // duration press of a button to timer reset

unsigned char button_state()
{
	/* the button is pressed when BUTTON bit is clear */
	if (!(PINC & (1<<BUTTON)))
	{
		_delay_ms(DEBOUNCE_TIME);
		if (!(PINC & (1<<BUTTON))) return 1;
	}
	return 0;
}
ISR (TIMER0_COMPA_vect)
{
	if (button_state()) // The button is held down
	{
		ms_button+=10;	// duration press of button is incremented
	}
	else
	{
		if (ms_button!=0) // The button is released
		{
			if (ms_button > RESET_BUTTON_DURATION) // if duration press of button is more than RESET_BUTTON_DURATION
			{
				state_button = 0; // reset timer
			}
			else
			{
				if (state_button == 1) state_button = 2; else state_button = 1; // if timer was paused then it should be start and vice versa.
			}
			ms_button=0;
		}
	}

	if (state_button!=1)
	{
		if (state_button==0) // reset timer
		{
			ms=0;
			sec=0;
			min=0;
			hr=0;
			state_button = 1;
		}
		else // timer counting
		{
			ms+=10; // increase a counter of milliseconds
			if (ms>999)
			{
				ms=0;
				sec++;// increase a counter of seconds
				if (sec>59)
				{
					sec=0;
					min++; // increase a counter of minutes
					if (min>59)
					{
						min=0;
						hr++; // increase a counter of hours
						if (hr>99)
						{
							hr=0;
						}
					}
				}
			}
		}
	}
}

uint16_t prawo = 0;
uint16_t lewo = 0;
uint16_t zakres = 30;

// Funkcja obs³uguj¹ca przerwanie od Timer2 (ISR)
ISR(TIMER2_OVF_vect) {
	
	lewo =odczytADC(PC3);
	prawo =odczytADC(PC4);
	
	if(lewo>prawo && (lewo-prawo)>zakres){
		if(OCR1A > 249){
			--OCR1A;
			_delay_ms(5);
		}
		
		}else if(prawo > lewo && (prawo - lewo)>zakres){
		if(OCR1A < 624){
			++OCR1A;
			_delay_ms(5);
		}
	}
}




int main(){
	
	uint16_t wynik=0;
	OCR1A = 437;
	
	lcd_init();
	
	DDRC &= ~(1<<PC0);//Makes 0 pin of PORTC as Input 
	PORTC|=_BV(PC0);
	//incjalizacja timerów
	timer1_conf();
	//timer2_conf();
	timer0_conf();
	//inicjalizacja ADC
	ADCSRA |=(1<<ADEN);//w³aczenie ADC
	ADCSRA |= _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);//presklaer na 128
	//ADCSRA |= (1<<ADPS2);//preskaler na 16
	ADMUX |= REF_11; //ustawianie wewnêtrznego Ÿród³a napiecia odniesiena 1.1V
	USART_Init(MYUBRR);
	
	
	while(1){
		
		if(!(PINC& _BV(PC0))==0) DDRD |= _BV(PD0);
		else DDRD &= ~_BV(PD0);
		unsigned int temp_int_2,temp_int_1,temp_int_0; // digits of counter
		wynik= pomiar(PC5);
		double konwersja = ADC_to_Temperature(wynik);
		int odczyt= (int)konwersja;
		
		lcd_gotoxy(0,0);
		lcd_printf("Temp %d C",odczyt);

	
		// determination the number of digits in a hour counter
		temp_int_1 = hr % 100 / 10;
		temp_int_0 = hr % 10;

		if(temp_int_1 > 0) // if result is double-digit - XX
		{
			//lcd_clear();
			//lcd_home();
			lcd_gotoxy(1,1);
			lcd_printf("%d",temp_int_1);
			lcd_gotoxy(4,1);
			lcd_printf("%d ",temp_int_0);
		}
		else // if result is single-digit - 0X
		{
			//lcd_clear();
			//lcd_home();
			lcd_gotoxy(4,1);
			lcd_printf("0");
			lcd_gotoxy(5,1);
			lcd_printf("%d ",temp_int_0);
		}

		// determination the number of digits in a minute counter
		temp_int_1 = min % 100 / 10;
		temp_int_0 = min % 10;

		if(temp_int_1 > 0) // if result is double-digit
		{
			//lcd_clear();
			//lcd_home();
			lcd_gotoxy(7,1);
			lcd_printf("%d",temp_int_1);
			lcd_gotoxy(8,1);
			lcd_printf("%d ",temp_int_0);
		}
		else // if result is single-digit
		{
			//lcd_clear();
			//lcd_home();
			lcd_gotoxy(7,1);
			lcd_printf("0");
			lcd_gotoxy(8,1);
			lcd_printf("%d ",temp_int_0);
			
		}

		// determination the number of digits in a second counter
		temp_int_1 = sec % 100 / 10;
		temp_int_0 = sec % 10;

		if(temp_int_1 > 0)
		{
			//lcd_clear();
			//lcd_home();
			lcd_gotoxy(10,1);
			lcd_printf("%d",temp_int_1);
			lcd_gotoxy(11,1);
			lcd_printf("%d ",temp_int_0);
		}
		else
		{
			//lcd_clear();
			//lcd_home();
			lcd_gotoxy(10,1);
			lcd_printf("0");
			lcd_gotoxy(11,1);
			lcd_printf("%d ",temp_int_0);
		}

		// determination the number of digits in a millisecond counter
		temp_int_2 = ms % 1000 / 100; // triple-digit
		temp_int_1 = ms % 100 / 10; // double-digit
		temp_int_0 = ms % 10; // single-digit
		
		if(temp_int_2 > 0) // if result is triple-digit number - XXX
		{
			//lcd_clear();
			//lcd_home();
			lcd_gotoxy(13,1);
			lcd_printf("%d",temp_int_2);
			lcd_gotoxy(14,1);
			lcd_printf("%d",temp_int_1);
			lcd_gotoxy(15,1);
			lcd_printf("%d ",temp_int_0);
			
		}
		else
		{
			if(temp_int_1 > 0) // if result is double-digit - 0XX
			{
				//lcd_clear();
				//lcd_home();
				lcd_gotoxy(13,1);
				lcd_printf("0");
				lcd_gotoxy(14,1);
				lcd_printf("%d",temp_int_1);
				lcd_gotoxy(15,1);
				lcd_printf("%d ",temp_int_0);

			}
			else // if result is single-digit - 00X
			{
				//lcd_clear();
				//lcd_home();
				lcd_gotoxy(13,1);
				lcd_printf("0");
				lcd_gotoxy(14,1);
				lcd_printf("0");
				lcd_gotoxy(15,1);
				lcd_printf("%d ",temp_int_0);
			}
		}
		
		
		//USART_put_P("%d\r\n",temp);
		/*OCR1A=999;
		_delay_ms(900);
		OCR1A=4999;
		_delay_ms(900);*/
		
		uint16_t prawo = 0;
		uint16_t lewo = 0;
		uint16_t zakres = 30;
		
		lewo =odczytADC(PC3);
		prawo =odczytADC(PC4);
		
		if(lewo>prawo && (lewo-prawo)>zakres){
			if(OCR1A > 249){
				--OCR1A;
				_delay_ms(1);
			}
			
			}else if(prawo > lewo && (prawo - lewo)>zakres){
			if(OCR1A < 624){
				++OCR1A;
				_delay_ms(1);
			}
		}
		
		print_to_USART("wartosc ADC %d\r\n",wynik);

		}

		}
