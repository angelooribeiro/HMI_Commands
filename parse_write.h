/*
 * parse_write.h
 *
 * Created: 23/02/2016 20:44:42
 *  Author: angel
 */ 


#ifndef PARSE_WRITE_H_
#define PARSE_WRITE_H_

#include "header.h"

extern void parse_write(void);
void write_integer(void);		//write an integer from a memory position
void write_digital(void);		//write a digital input
void write_adc(void);			//write the input from adc
void write_e2prom(void);			//write a addr from e2prom



#endif /* PARSE_WRITE_H_ */