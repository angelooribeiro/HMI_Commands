/*
 * paerse_read.h
 *
 * Created: 23/02/2016 16:34:23
 *  Author: angel
 */ 


#ifndef PARSE_READ_H_
#define PAESE_READ_H_

#include "header.h"

extern void parse_read(void);
void read_integer(void);		//read an integer from a memory position
void read_digital(void);		//read a digital input
void read_adc(void);			//read the input from adc
void read_e2prom(void);			//read a addr from e2prom


#endif /* PAERSE_READ_H_ */