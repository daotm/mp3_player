/*
 * File:   SPI.c
 * Author: TD
 *
 * Created on February 22, 2020, 9:39 AM
 */



#include "SPI.h"



/*
 * FOSC = FIN * M/(N1*N2)
 * M = PLLDIV + 2               REGISTER PLLFBD
 * N1 = PLLPRE + 2              REGISTER CLKDIV
 * N2 = 2*(PLLPOST+1)           REGISTER CLKDIV
 * 
 * The PFD input frequency (FPLLI) must be in the range of 0.8 MHz to 8.0 MHz
 * The VCO output frequency (FSYS) must be in the range of 120 MHz to 340 MHz
 * 
 * FPLLI = FIN/N1
 * FSYS = FIN * M/N1
 * 
 * PLLFBD = PLLDIV => M = PLLFBD + 2
 * 
 * Setup 40MHz FOSC: M = 38, N1=2, N2=4;
 * FPLLI = 8MHZ/2 = 4MHZ.
 *  FSYS = 8MHZ*38/2 = 152MHz
 */

void clk_SW()
{
    //FOSC = 40MHZ
    
    PLLFBD=38; // M=40
    CLKDIVbits.PLLPOST=1; // N2=4
    CLKDIVbits.PLLPRE=0; // N1=2
    
    // Initiate Clock Switch to Primary Oscillator with PLL (NOSC=0b011)
    __builtin_write_OSCCONH(0x03);
    __builtin_write_OSCCONL(OSCCON | 0x01);
    // Wait for Clock switch to occur
    while (OSCCONbits.COSC!= 0b011);
    // Wait for PLL to lock
    while (OSCCONbits.LOCK!= 1);
}




void SPI_init()
{
    //_SS2R = 125;    
    //RPINR23bits.SS2R = 125;            // remap /SS pin to RG13/RP125
    
//    TRISGbits.TRISG9 = 0;               //SD-CS already defined in SD.h
//    LATGbits.LATG9 = 1;                 //
//    

    TRISGbits.TRISG6 =0;                //SDK clock out
    TRISGbits.TRISG7 = 1;               //MISO
    TRISGbits.TRISG8 = 0;               //MOSI 
    ANSELG = 0x0000;               //
    
    SPI2CON1bits.MSTEN = 1;     // Master mdoe Mode
    SPI2CON1bits.MODE16 = 0;    //8 bit wide comm
    SPI2CON1bits.SMP = 0;       // must be clear in slave mode
    
    //SPI Mode 3
//    SPI2CON1bits.CKE = 0;       // data changed on idle to active clock edge (1 to 0)
//    SPI2CON1bits.CKP = 1;       // Idle state for clock is high level
    
//    //SPI Mode 0
    SPI2CON1bits.CKE = 1;       // data changed on active to idle clock edge (1 to 0)
    SPI2CON1bits.CKP = 0;       // Idle state for clock is low level
    
//    //SPI Mode 1
//    SPI2CON1bits.CKE = 0;       // data changed on active to idle clock edge (1 to 0)
//    SPI2CON1bits.CKP = 0;       // Idle state for clock is low level
//    
//        //SPI Mode 4
//    SPI2CON1bits.CKE = 1;       // data changed on active to idle clock edge (1 to 0)
//    SPI2CON1bits.CKP = 1;       // Idle state for clock is high level
//    
    
    
// state is a high level
    SPI2CON1bits.SSEN = 0;      // /SS pin not used for module
    SPI2CON1bits.SPRE = 0;
    SPI2CON1bits.PPRE = 1;      // FSPI = FP/16 = 4MHz/16 = 250kHz
    
    
    SPI2CON2 = 0x0000;
    
    SPI2STATbits.SPIROV = 0;
    SPI2STATbits.SPIEN = 1;     //enable spi bus
    
    //disable interrupt
    //IFS2bits.SPI2IF = 0;        //clear IF flag
    //IEC2bits.SPI2IE = 1;        //enable SPI2 interrupt
    
    
    // still need global interrupt enabled.
}


void SPI_FREQ_SW()
{
    SPI2STATbits.SPIEN = 0;     //disable spi bus
    SPI2CON1bits.PPRE = 2;      // FSPI = FP/4 = 20MHz/4 = 5MHz
    SPI2STATbits.SPIEN = 1;     //enable spi bus
}



void SPI_write_byte(unsigned char val)
{
    unsigned char c;
    SPI2BUF = val;
    //while(SPI2STATbits.SPITBF);
    while(!SPI2STATbits.SPIRBF); //wait until all bit transferred.
    c = SPI2BUF;                    //read byte to prevent overflow.
}

unsigned char SPI_read_byte()
{
    unsigned char c;
    SPI2STATbits.SPIROV = 0;            //clear SPIROV
    SPI2BUF = 0xFF;
    while(!SPI2STATbits.SPIRBF); //wait until all bit transferred.
    c = SPI2BUF;
    return c;
}