/*
 * parse_write.c
 *
 * Created: 23/02/2016 20:44:25
 *  Author: angel
 */ 

#include "parse_write.h"

void parse_write(void);
void read_integer(void);		//write an integer from a memory position
void read_digital(void);		//write a digital input
void read_adc(void);			//write the input from adc
void write_e2prom(void);			//write a addr from e2prom

extern char parse_read();
extern char str_read();
extern long int parse_number();
extern long int parse_number_digital();

void parse_write()
{
	char temp = str_read();
	switch(temp)
	{
		//write integer
		case 'i':
		case 'I':
			temp = str_read();
			if(temp == 0x20)
				write_integer();
			if(temp != 0x20)
				serial_error();
			break;
		
		//write digital
		case 'd':
		case 'D':
			temp = str_read();
			if(temp == 0x20)
				write_digital();
			if(temp != 0x20)
				serial_error();
			break;
		
		//write e2prom
		case 'e':
		case 'E':
			temp = str_read();
			if(temp == 0x20)
				write_e2prom();
			if(temp != 0x20)
				serial_error();
			break;
		
		//error
		default:
			serial_error();
		break;
	} //end switch
}//end function

//write an integer from a memory position	#############################done and tested######################################
void write_integer(void)
{
	long int addr = parse_number();						//read the addr
	unsigned char control_digit1 = str_read_last();		//control digit
	long int value = parse_number();					//read the value
	unsigned char control_digit2 = str_read();

	if( (addr < 0x200)  || (addr > (0x21FF-1)) || (control_digit1 != 0x20) || (control_digit2 != '\0') || (value > 0xFFFF) )				//check if addr and value is invalid
		serial_error();
	
	if(  (addr >= 0x200)  && (addr <= (0x21FF-1)) && (control_digit1 == 0x20) && (control_digit2 == '\0') && (value <= 0xFFFF) )	
	{
		serial_writestr("Write integer to 0x");
		conv_hexa_ascii(&addr,2);
		serial_writestr(" -> ");
		long int *p = (long int*) addr;		//addr on a pointer
		unsigned char* q = p;
		(*q) = ((value&0xFF00)>>8);			//write the msb
		conv_hexa_ascii(q,1);					
		q++;										//inc the pointer to the next byte		
		(*q) = (value&0x00FF);						//write the lsb
		conv_hexa_ascii(q,1);
		save_command();
		serial_write('\n');
	}
	
}//end function

//write a digital input					####################################done and tested ###########################################
void write_digital(void)									/* PORTA = 0x00; PORTC = 0x02;			not defined for others ports*/
{
	digital_flag = 1;
	
	long byte_addr = parse_number();
	unsigned char control_digit1 = str_read_last();
	long bit_addr = parse_number_digital();
	unsigned char control_digit2 = str_read_last();
	unsigned char bit_value = (str_read()-48);
	unsigned char control_digit3 = str_read();
	
	if( ((byte_addr != 0) && (byte_addr != 2)) || (bit_addr <0)  || (bit_addr > 7) || (bit_value <0)  
					|| (bit_value > 1) || (control_digit1 != '.') || (control_digit2 != 0x20) || (control_digit3 != '\0') )
		serial_error();

	if( ((byte_addr == 0) || (byte_addr <= 0xFF)) && (bit_addr >= 0)  && (bit_addr <= 7) && (bit_value >= 0) 
						 && (bit_value <= 1) && (control_digit1 == '.') && (control_digit2 == 0x20) && (control_digit3 == '\0') )
	{
		unsigned char port_number = byte_addr + 65;    // gets ascii from port based on address
		unsigned char pin_number = bit_addr + 48;    // gets ascii from pin number
		serial_writestr("Write digital P_");
		serial_write(port_number);
		serial_writestr(".");
		serial_write(pin_number);
		serial_writestr(" -> ");
		serial_write(bit_value+48);
		unsigned int *p = (unsigned int*) ((byte_addr*3)+0x22);
		if(bit_value == 1)
		{
			DDRA = 1;
			DDRC = 1;
			(*p) |= (1 << (bit_addr)); //set bit
		}
		if(bit_value == 0)
		{	
			DDRA = 0;
			DDRC = 0;
			(*p) &= ~(1 << (bit_addr)); // clear bit
		}
		save_command();
		serial_write('\n');
	}
	digital_flag = 0;
}//end function

//write a addr from e2prom		#####################/*done and tested*/#########################################
void write_e2prom(void)
{
	long int addr = parse_number();
	unsigned char control_digit1 = str_read_last();
	long int value = parse_number();
	unsigned char control_digit2 = str_read();
	//4kbytes eeprom memory
	if( (addr < 0)  || (addr > 0xFFF) || (control_digit1 != 0x20) || (control_digit2 != '\0') || (value > 0xFF) || (value < 0x00) )					//verifies if isn't a valid addr
	serial_error();
	if( (addr >= 0)  && (addr <= 0xFFF) && (control_digit1 == 0x20) && (control_digit2 == '\0') && (value <= 0xFF) && (value >= 0x00))					//verifies if is a valid addr
	{
		serial_writestr("E2PROM write to 0x");
		conv_hexa_ascii(&addr,2);
		serial_writestr(" -> ");
		conv_hexa_ascii(&value,1);
		EEAR = addr;						//addr to be written
		EEDR = value;						//value to write on addr
		EECR |= (1<<EEMPE);					//enable write operation
		EECR |= (1<<EEPE);					//start write operation
		while(EECR & (1<<EEPE));			//EEPE is clear by hardware after 4 machine cycles, write done
		save_command();
		serial_write('\n');
	}
	
}//end function