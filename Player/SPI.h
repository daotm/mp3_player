/* 
 * File:   SPI.h
 * Author: TD
 *
 * Created on February 22, 2020, 9:39 AM
 */

#ifndef SPI_H
#define	SPI_H

#include "xc.h"
#include <p24EP512GU810.h>


void SPI_init();
void clk_SW();
void SPI_write_byte(unsigned char val);
unsigned char SPI_read_byte();
void SPI_FREQ_SW();



#endif	/* SPI_H */

