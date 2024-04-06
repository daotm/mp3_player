/*
 * File:   main.c
 * Author: TD
 *
 * Created on February 15, 2020, 10:46 AM
 */


#include "xc.h"
#include "config.h"
#include "HardwareProfile.h"
#include "Graphics.h"
#include "TouchScreen.h"
#include "LCDTerminal.h"
#include <stdio.h>
#include "TouchScreenResistive.h"
#include "DownBtn.h"
#include "PlayBtn.h"
#include "UpBtn.h"
#include "Smiley.h"
#include "SPI.h"
#include "CONU2.h"
#include "SD.h"
#include "VS1053.h"
#include "player.h"




#define __ISR   __attribute__((interrupt,shadow,no_auto_psv))

void __ISR _T3Interrupt(void)
{
    _T3IF = 0;
    TouchDetectPosition();
}

#define TICK_PERIOD(ms) (GetPeripheralClock()*(ms))/8000
void TickInit(unsigned period_ms)
{
    TMR3=0;
    PR3=TICK_PERIOD(period_ms);
    T3CONbits.TCKPS=1;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE=1;
    T3CONbits.TON=1;
}

void TMR5Init()
{
    TMR5=0;
    T5CONbits.TCKPS=3;
    IFS1bits.T5IF = 0;
    IEC1bits.T5IE = 0;
    T5CONbits.TON = 1;
}

void wait_H(unsigned int value)
{
    TMR5=0;
    while(TMR5 < value);
}


unsigned char temp;
unsigned char SD_init_val;
unsigned char VS_init_val;



int main(void) {

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

    RECORD * rec_ptr;
    rec_ptr = init_playlist(SD_ptr);            //point to the first song on the playlist
   
    
    
    
    
    int width;
    int btnW,btnH,btn2H,btn2W;

    LCDInit();

    TickInit(1);
    DisplayBacklightOn();
    //TouchInit(NULL,NULL,NULL,NULL);
    TouchHardwareInit(NULL);
    TouchCalculateCalPoints();
    TMR5Init();

    SetFont((void*) &GOLFontDefault);
    
    btnW = GetImageWidth((void*) &DownBtn);               //same width for all bit map
    btnH = GetImageHeight((void*) &DownBtn);               //Down button
    btn2W = GetImageWidth((void*) &PlayBtn);               //same width for all bit map
    btn2H = GetImageHeight((void*) &PlayBtn);               //same width for all bit map
    
    IMAGE_FLASH BtnPlay = PlayBtn;
    IMAGE_FLASH BtnDown = DownBtn;
    IMAGE_FLASH BtnUp = UpBtn;
    IMAGE_FLASH TSmiley = Smiley;

    LCDClear();

    SetColor(YELLOW);
    ClearDevice();

    unsigned char t = 0;
    LCDSetBackground(YELLOW);
    LCDSetXY(10, 5);
    for(t=0; t<8; t++)
        LCDPut(rec_ptr->name[t]);
    unsigned int Up_X, Up_Y, Down_X, Down_Y, Play_X, Play_Y;
    
    Up_X = 15;
    Up_Y = 10;
    Play_X = 8;
    Play_Y = 80;
    Down_X = 15;
    Down_Y = 165;

    
    PutImage(Up_X, Up_Y, (void*) &BtnUp, IMAGE_NORMAL);
    PutImage(Play_X, Play_Y, (void*) &BtnPlay, IMAGE_NORMAL);
    PutImage(Down_X, Down_Y, (void*) &BtnDown, IMAGE_NORMAL);
    unsigned int x, y, song_ptr_offset;
    song_ptr_offset = 0;
    while(1)
    {
        x = TouchGetX();
        y = TouchGetY();
        if((x != -1) && (y != -1))
        {
            if(x > Up_X && x <(Up_X + btnW) && y > Up_Y && y <(Up_Y + btnH))         //UP button
            {
                TMR5 = 0;
                while(TMR5 < 30000);                
                if(song_ptr_offset < get_playlist_sz()-1)
                    song_ptr_offset = song_ptr_offset + 1;
                
                LCDSetXY(10, 5);
                for(t=0; t<8; t++)
                    LCDPut((rec_ptr+song_ptr_offset)->name[t]);
                TMR5 = 0;
                while(TMR5 < 50000);
            } 
            else if(x > Play_X && x <(Play_X + btn2W) && y > Play_Y && y <(Play_Y + btn2H)) //Play button resp
            {
                TMR5 = 0;
                while(TMR5 < 30000);       
                
                play_song(rec_ptr+song_ptr_offset);
                
                
                TMR5 = 0;
                while(TMR5 < 30000);       
                
                
                
//                
//                LCDPutString("Baby Steps");
//                TMR5 = 0;
//                while(TMR5 < 50000);                 
//                LCDSetXY(7, 5);
//                LCDPutString("into");
//                TMR5 = 0;
//                while(TMR5 < 50000);                 
//                LCDSetXY(7, 7);
//                LCDPutString("Giant Strides");
//                
                
                
            }
            else if(x > Down_X && x <(Down_X + btnW) && y > Down_Y && y <(Down_Y + btnH))
            {
                TMR5 = 0;
                while(TMR5 < 30000);
                //SetColor(YELLOW);
                //ClearDevice();
                
                if(song_ptr_offset > 0)
                    song_ptr_offset = song_ptr_offset - 1;
                
                LCDSetXY(10, 5);
                for(t=0; t<8; t++)
                    LCDPut((rec_ptr+song_ptr_offset)->name[t]);
                
                TMR5 = 0;
                while(TMR5 < 50000);
            }
            
        }
        
    }
}
