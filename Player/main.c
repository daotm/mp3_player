/*
 * File:   main.c
 * Author: TD
 *
 * Created on February 22, 2020, 9:25 AM
 */


#include "xc.h"
#include "config.h"
#include <p24EP512GU810.h>
#include "SPI.h"
#include "CONU2.h"
#include "SD.h"
#include "VS1053.h"
#include "player.h"
//interrupt handler for transferring data over uart
//note that SPI bus must be much faster than UART
//char temp;
//
//void __attribute__((__interrupt__,__auto_psv__)) _SPI2Interrupt(void)
//{
//    temp = SPI2BUF;                 //read from SPI Buff
//    putU2(temp);
//    IFS2bits.SPI2IF = 0;
//}
unsigned char temp;
unsigned char SD_init_val;
unsigned char VS_init_val;
//unsigned char SD_BUF[512];

int main(void) {
    //InitU2();
    //putsU2("U2 init completed");
    SPI_init();
    T3CONbits.TCKPS = 3;
    T3CONbits.TCS = 0;
    T3CONbits.TON = 1;
    
    TRISDbits.TRISD2 = 0;           //turn off LED
    ANSELD = 0x00;
    LATDbits.LATD2 = 0;
    
    
    SD_INFO * SD_ptr;
    SD_ptr = SD_init();
    
    asm("nop");
    TMR3 = 0;
    while(TMR3 < 4000);       //wait 250ms for SD card to settle.
    VSTestInitHardware();
    VS_init_val = VSTestInitSoftware();
    
    
    if(SD_ptr->init_code != 0 || VS_init_val != 0)
        while(1);                                   //trap error;
        
    clk_SW();               //perform clock switch to 40MHz
    SPI_FREQ_SW();          //perform SPI clock switch to 4MHz
    while(TMR3 < 4000);       //wait 250ms for SD card to settle.
    //SD_Read_Block(&SD_BUF[0],32752);
    //testMP3Sine(0x44);
    //play_song();
    RECORD * rec_ptr;
    rec_ptr = init_playlist(SD_ptr);
    
    play_song(rec_ptr+4);
    

    while(1);
}


