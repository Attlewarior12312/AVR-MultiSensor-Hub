

#define set_lcd_d4 PORTB|=(1<<2)  //PB2
#define clr_lcd_d4 PORTB&=~(1<<2)
#define ddr_d4 DDRB|=(1<<2)           // tak jak set_lcd

#define set_lcd_d5 PORTB|=(1<<3) //PB3
#define clr_lcd_d5 PORTB&=~(1<<3)
#define ddr_d5 DDRB|=(1<<3)

#define set_lcd_d6 PORTB|=(1<<4) //PB4
#define clr_lcd_d6 PORTB&=~(1<<4)
#define ddr_d6 DDRB|=(1<<4)

#define set_lcd_d7 PORTB|=(1<<5)  //PB5
#define clr_lcd_d7 PORTB&=~(1<<5)
#define ddr_d7 DDRB|=(1<<5)

#define set_lcd_en PORTB|=(1<<0) ////PB0
#define clr_lcd_en PORTB&=~(1<<0)
#define ddr_en DDRB|=(1<<0)

#define set_lcd_rs PORTD|=(1<<7) //PD7
#define clr_lcd_rs PORTD&=~(1<<7)
#define ddr_rs DDRD|=(1<<7)

// podswietlenie , jesli uzywane
#define set_lcd_light PORTC|=(1<<6)
#define clr_lcd_light PORTC&=~(1<<6)
#define ddr_light DDRC|=(1<<6)




void lcd_init(void);
void lcd_send_data(unsigned char b);
void lcd_send_com(unsigned char b);
void lcd_send_4(unsigned char b);
void lcd_send_8(unsigned char b);
void lcd_control( unsigned char on, unsigned char cur, unsigned char blink);
void lcd_clear(void);
void lcd_home(void);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_swrite( char *s);
void lcd_iwrite(int i);
int lcd_printf( char *format, ... );


