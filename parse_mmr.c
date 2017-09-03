/*
 * memory_opr.c
 *
 * Created: 23/02/2016 22:41:16
 *  Author: angel
 */ 
#include "parse_mmr.h"

void parse_memory(void);
void copy_memory(void);			//copy one segment of memory
void read_memory(void);			//read a segment of memory
void write_memory(void);		//write in a segment of memory

//external functions
extern char str_read();
extern long int parse_number();
extern void conv_hexa_ascii(unsigned char* conv, char nbytes);

//0x200-21ff Internal SRAM only possible write here directly 

void parse_memory(void)
{
	char temp = str_read();
	switch(temp)
	{
		//read
		case 'r':
		case 'R':
			temp = str_read();
			if(temp == 0x20)
				read_memory();
			if(temp != 0x20)
				serial_error();
			break;
		
		//copy
		case 'c':
		case 'C':
			temp = str_read();
			if(temp == 0x20)
				copy_memory();
			if(temp != 0x20)
				serial_error();
			break;
		
		//write
		case 'w':
		case 'W':
			temp = str_read();
			if(temp == 0x20)
				write_memory();
			if(temp != 0x20)
				serial_error();
			break;
		
		//error
		default:
			serial_error();
			break;
	} //end switch
}//end function

//copy one segment of memory					#########################done and tested#######################################
void copy_memory(void)
{	
	DDRA = 1;
	PORTA |= 0x01;
	long int org = parse_number();		//origin addr
	unsigned char control_digit1 = str_read_last();	//read control character
	long int lenght = parse_number();	//length to be copied
	unsigned char control_digit2 = str_read_last();	//read control character
	long int dest = parse_number();		//destiny addr
	unsigned char control_digit3 = str_read();
	
	if ( (org<0x200) || (dest<0x200) || (lenght<=0) || ((lenght+org)>0x21FF) || ((lenght+dest)>0x21FF) 
					|| (control_digit1 != 0x20) || (control_digit2 != 0x20) || (control_digit3 != '\0') )	//if checks some error
		serial_error();
		
	if( (org>=0x200) && (dest>=0x200) && (lenght>0) && ((lenght+org)<=0x21FF) && ((lenght+dest)<=0x21FF) 
							&& (control_digit1 == 0x20) && (control_digit2 == 0x20) && (control_digit3 == '\0') ) //if all checks
	{
		serial_writestr("Memmory copy from 0x");
		conv_hexa_ascii(&org,2);
		serial_writestr(" to 0x");
		conv_hexa_ascii(&dest,2);
		serial_write('(');
		conv_hexa_ascii(&lenght,2);
		serial_writestr(" bytes)");
		unsigned char *po = (unsigned char*) org;		//conversion to a pointer that points to data to be copied
		unsigned char *pd = (unsigned char*) dest;		//pointer for the addr that receive data
		
		for (char i = 0; i < lenght; i++, po++, pd++){	//copies data
			(*pd)= *po;
			conv_hexa_ascii(po,1);
			}
		save_command();									//save the command executed with success
		serial_write('\n');
	}
	PORTA &= (~0x01);
}//end function

//read a segment of memory		#################################done and tested###################################
void read_memory(void)
{
	long int addr = parse_number();					//byte addr
	unsigned char control_digit1 = str_read_last();		//read the digital control
	long int lenght = parse_number();				//length to be redden
	unsigned char control_digit2 = str_read();
	
	if( (addr < 0x200) || (lenght <= 0) || ( (lenght+addr-1) > 0x21FF) || (control_digit1 != 0x20) || (control_digit2 != '\0') )	//if some one fails
		serial_error();									//print an error message

	if( (addr >= 0x200)  && (lenght > 0) && ( (lenght+addr-1) <= 0x21FF) && (control_digit1 == 0x20) && (control_digit2 == '\0') )	//if check all conditions
	{
		serial_writestr("Memmory read from 0x");
		conv_hexa_ascii(&addr,2);
		serial_write('(');
		conv_hexa_ascii(&lenght,2);
		serial_writestr(" bytes) -> ");
		unsigned char *p = (unsigned char*) addr;
		for (char i = 0; i < lenght; i++, p++) 
		conv_hexa_ascii(p,1);		//prints the reeded value
		save_command();			//save the command executed with success 
		serial_write('\n');
	}
}//end function

//write in a segment of memory				#################################done and tested###############################################
void write_memory(void)
{
	long int addr = parse_number();				//byte addr
	unsigned char control_digit1 = str_read_last();	//read control character
	long int lenght = parse_number();		//length to be written
	unsigned char control_digit2 = str_read_last();	//read control character
	long int byte = parse_number();		//byte to be written
	unsigned char control_digit3 = str_read();
	
	if( (addr<0x200) || ( (addr+lenght-1)>0x21FF) || (lenght<0) || (byte<0) || (byte>0xFF) 
			|| (control_digit1 != 0x20) || (control_digit2 != 0x20) || (control_digit3 != '\0') ) //if some one fails
		serial_error();							//print an error message
	
	if( (addr >= 0x200) && ( (addr+lenght-1) <= 0x21ff) && (lenght > 0) && (byte >= 0) && 
			(byte <= 0xFF) && (control_digit1 == 0x20) && (control_digit2 == 0x20) && (control_digit3 == '\0') )		//if all conditions are verified 
	{
		serial_writestr("Memmory write to 0x");
		conv_hexa_ascii(&addr,2);
		serial_write('(');
		conv_hexa_ascii(&lenght,2);
		serial_writestr(" bytes) -> ");
		unsigned char *p = (unsigned char*) addr;		//puts the addr in the pointer								
		for (char i = 0; i < lenght; i++, p++){			//makes the cycle how many times there is a position
			(*p)= byte;									//writes in memory
			conv_hexa_ascii(p,1);	
		}								
		save_command();									//save the command executed with success
		serial_write('\n');
	}
}//end function