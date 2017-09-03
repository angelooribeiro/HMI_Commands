/*

 * paerse_read.c
 *
 * Created: 23/02/2016 16:34:44
 *  Author: angel
 */ 
#include "parse_read.h"

void parse_read(void);
void read_integer(void);		//read an intiger from a memory position
void read_digital(void);		//read a digital input
void read_adc(void);			//read the input from adc
void read_e2prom(void);			//read a addr from e2prom

extern char str_read();
extern char str_read_last();
extern long int parse_number();
extern long int parse_number_digital();
extern void conv_hexa_ascii(unsigned char* conv, char nbytes);
extern void conv_adc_ascii();

extern unsigned char digital_flag;

//read operations
void parse_read()
{
	char temp = str_read();
	switch(temp)
	{
		//read integer
		case 'i':
		case 'I':
			temp = str_read();
			if(temp == 0x20)
				read_integer();
			if(temp != 0x20)
				serial_error();
			break;
	
		//read digital
		case 'd':
		case 'D':
			temp = str_read();
			if(temp == 0x20)
				read_digital();
			if(temp != 0x20)
				serial_error();
			break;
	
		//read analog adc
		case 'a':
		case 'A':
			temp = str_read();
			if(temp == 0x20)
				read_adc();		//prints the value on the adc
			if(temp != 0x20)
				serial_error();
			break;
	
		//read e2prom
		case 'e':
		case 'E':
			temp = str_read();
			if(temp == 0x20)
				read_e2prom();
			if(temp != 0x20)
				serial_error();
			break;
	
		//error
		default:
			serial_error();
		break;
	} //end switch
}//end function

//read an integer from a memory position    ##############################done and tested##############################
void read_integer(void)
{
	long int addr = parse_number();
	unsigned char control_digit = str_read();
	
	//external sram access only
	if( (addr < 0x200)  || (addr > (0x21FF-1)) || (control_digit != '\0') )				//verifies if isn't a valid addr
		serial_error();																	//outputs an error message
		
	if( (addr >= 0x200) && (addr <= (0x21FF-1)) && (control_digit == '\0') )		
	{
		serial_writestr("Read integer from 0x");
		conv_hexa_ascii(&addr,2);
		serial_writestr(" -> ");
		long int *p = (long int*) addr;				//conversion for memory pointer
		unsigned char* q = p;
		conv_hexa_ascii(q,1);						//returns the integer: lsb
		q++;
		conv_hexa_ascii(q,1);					//msb
		save_command();
		serial_write('\n');
	}
}

//read a digital input					################################done and tested##################################################
void read_digital(void)
{											/* PORTA = 0x00; PORTC = 0x02;			not defined for others ports*/
	digital_flag = 1;						//indicates in parse that is a different read opration
	long byte_addr = parse_number();
	unsigned char control_digit1 = str_read_last();
	long bit_addr = parse_number_digital();
	unsigned char control_digit2 = str_read();
	
	if( ((byte_addr != 0x00) && (byte_addr != 0x02)) || (bit_addr < 0) || (bit_addr > 7) || (control_digit1 != '.') || (control_digit2 != '\0') )
		serial_error();
	
	if( ((byte_addr == 0x00) || (byte_addr == 0x02)) && (bit_addr >= 0) && (bit_addr <= 7) && (control_digit1 == '.') && (control_digit2 == '\0') )
	{
		unsigned int *p = (unsigned int*)((byte_addr*3)+0x20);				//makes conversion for the addr on memory
		unsigned char port_number = byte_addr + 65;    // gets ascii from port based on address
		unsigned char pin_number = bit_addr + 48;    // gets ascii from pin number
		serial_writestr("Read digital P_");
		serial_write(port_number);
		serial_writestr(".");
		serial_write(pin_number);
		serial_writestr(" -> ");
		if(((*p)& (1 << (bit_addr))))
			serial_write('1');
		if( ! ((*p) & (1 << (bit_addr)) ) )
			serial_write('0');
		save_command();
		serial_write('\n');
	}
	digital_flag = 0;
}

//read the input from adc				#####################the first conversion is done, the others take the same value#########################
void read_adc(void)							
{	
	DDRA = 1;
	PORTA |= 0x01;
	long int selected_pin = parse_number();
	unsigned char control_digit = str_read();
		
	if( (selected_pin < 0x00) || (selected_pin > 0x07) || (control_digit != '\0') )			//the addr of adc ports goes to 0 to 7
		serial_error();										//error message

	if( (selected_pin >= 0x00) && (selected_pin <= 0x07) && (control_digit == '\0') )	
	{
	    unsigned char adc[2];
		ADMUX = 0;
		ADMUX = (1 << REFS0);						
		ADMUX |= selected_pin;						//select the pin to be read
		ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));    //Prescaler at 128 so we have an 125Khz clock source
		ADCSRA |= (1 << ADEN);						//adc enables, start conversion
		ADCSRA |= (1 << ADSC);						//this write operation must have an interval of one cycle clock
		
		while( (ADCSRA & (1<<ADIF)) != 0x10 );				//waits conversion
		conv_adc_ascii();
		serial_write('\n');
		ADCSRA |= (1 << ADIF);						//clear the flag
		ADCSRA = 0;						//disable adc
		
		save_command();
	}
	PORTA &= (~0x01);
}//end function

//read a addr from e2prom		#####################/*done and tested*/#########################################
void read_e2prom(void)
{
	long int addr = parse_number();
	unsigned char control_digit1 = str_read_last();
	long int length = parse_number();
	unsigned char control_digit2 = str_read();
																	//4kbytes eeprom memory
	if( (addr < 0)  || ((addr+length-1) > 0xFFF) || (length <= 0) || (control_digit1 != 0x20) || (control_digit2 != '\0') )						//verifies if isn't a valid addr
		serial_error();
	
	if( (addr >= 0) && ((addr+length-1) <= 0xFFF) && (length > 0) && (control_digit1 == 0x20) && (control_digit2 == '\0') )						//verifies if is a valid addr	
	{
		serial_writestr("E2PROM read from 0x");
		conv_hexa_ascii(&addr,2);
		serial_write('(');
		conv_hexa_ascii(&length,1);
		serial_writestr(" bytes) -> ");
		for(char i = 0; i < length; i++)
		{
			EEAR = addr;					//addr to be redden
			EECR |= (1<<EERE);				//start reading, takes only one machine cycle
			unsigned char temp = EEDR;
			conv_hexa_ascii(&temp,1);				//transmit the reeded byte
			addr++;							//inc the pointer to thge next position of memory 	
		}
		save_command();
		serial_write('\n');
	}
	
}//end function

