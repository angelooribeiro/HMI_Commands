/*
 * CFile1.c
 *
 * Created: 22/02/2016 22:13:04
 *  Author: angel
 */ 
extern volatile char parse_index;
extern volatile char received_comand[];					//command to be executed
extern char str_read(void);
extern char version[];
extern void save_command();
extern void parse_read();
extern void parse_write();
extern void parse_memory();

unsigned char digital_flag = 0;

#include "header.h"

void parse(void)
{
	char temp = str_read();
	switch(temp)
	{
		//request for read
		case 'r':
		case 'R':
			parse_read();
			break;
		
		//request for write 
		case 'w':
		case 'W':
			parse_write();
			break;
		
		//request for memory acess
		case 'm':
		case 'M':
			parse_memory();
			break;
		
		//request for printing the version
		case'v':
		case 'V':
			temp = str_read();
			if( (temp != 'e') && (temp != 'E') )
				serial_error();
			if( (temp =='e') || (temp =='E') )
			{
				temp  = str_read();
				if( (temp != 'r') && (temp != 'R') )
					serial_error();
				if( (temp == 'r') || (temp == 'R') )
				{
					serial_writestr(version);
					save_command();				//save the last command executed with success
				}
			}
			break;
		
		//if nothing checks
		default:
			serial_error();
			break;
		
	}
	
}
