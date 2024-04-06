/*
 * File:   SD.c
 * Author: TD
 *
 * Created on March 21, 2020, 2:05 PM
 */



#include "SD.h"

unsigned char RES_BUF[8];
unsigned char ARG[4];
unsigned char TEMP;
SD_INFO SD_CARD;

SD_INFO * SD_init()
{
    SD_CARD.init_code = 89;                     //fail init code
    SD_CARD.BPB_OFFSET = 2048;
    SD_CARD.SECTOR_SZ = 512;
    SD_CARD.CLUSTER_SZ = 32;
    SD_CARD.FAT_SZ = 15256;                    //fat table size 32 sector
    SD_CARD.RESERVED_SZ = 32;
    SD_CARD.FAT_NUM = 2;
    SD_CARD.FIRST_DAT_SECTOR = SD_CARD.BPB_OFFSET + SD_CARD.RESERVED_SZ + (SD_CARD.FAT_SZ * SD_CARD.FAT_NUM); 

    
    unsigned char RESP;
    
    TRISGbits.TRISG9 = 0;
    ANSELGbits.ANSG9 = 0; 
    SD_CS = 1;      
    
    //Send 80 CLK while keeping data high and CS high to get SD card in to SD mode
    unsigned char i=0;
    while(i<13)
    {
        SPI_write_byte(0xFF);
        i++;
    }
    
    //send CMD0 () ////////////////////////////////
    RESP = SD_CMD(0x40,0x00,0x00,0x00,0x00,0x95);
    if(RESP != 0x01)
    {
        //putsU2("CM0 fail \n");
        asm("nop");
        SD_CARD.init_code = 40;
        return &SD_CARD;
        //return 40;
    }
    else
    {
        asm("nop");
        //putsU2("CMD0 pass \n");
    }
        

    //send CMD8 ()////////////////////////////////////////
    RESP = SD_CMD(0x48,0x00,0x00,0x01,0xAA,0x87);
    if(RESP == 0x01)
    {
        asm("nop");
        //putsU2("V1 SD card \n ");
    }
    else if(RESP == 0x05)
    {
        asm("nop");
        //putsU2("V2 pass \n ");
    }
    else 
    {
        //putsU2("CM8 fail \n");
        SD_CARD.init_code = 8;
        return &SD_CARD;
        //return 8;
    }
    
     //send CMD59 ()////////////////////////////////////////
    // Disable CRC check
    RESP = SD_CMD(0x7B,0x00,0x00,0x00,0x00,0x91);    
    if(RESP == 0x01)
    {
        asm("nop");
        //putsU2("IDLE \n");
    }
    else 
    {
      //putsU2("CM59 fail \n");
        SD_CARD.init_code = 59;
        return &SD_CARD;
        //return 0x59;
    }
    
    //Send CMD58 check content of the OCR operating register
    RESP = SD_CMD(0x7A,0x00,0x00,0x00,0x00,0x00); 
    
    
    
    //ACMD41 loop
    //Send CMD55 followed by CMD41 until 0x00 response is received
    while(1)
    {
        TEMP = SPI_read_byte();
        RESP = SD_CMD(0x77,0x00,0x00,0x00,0x00,0xFF);  
        if(RESP == 0x01)
        {
            asm("nop");
            //putsU2("IDLE \n");
        }
        else 
        {
            //putsU2("CM55 fail \n");
            SD_CARD.init_code = 55;
            return &SD_CARD;
            //return 0x55;
        }
        TEMP = SPI_read_byte();
        RESP = SD_CMD(0x69,0x40,0x00,0x00,0x00,0xFF);  
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        if(RESP == 0x00)
        {
//            putsU2("SD Init completed \n");
            break;
        }
        else if(RESP==0x01)
        {
            asm("nop");
            //            putsU2("SD initializing... \n");
        }
        else 
        {
            SD_CARD.init_code = 44;
            return &SD_CARD;
            //return 0x044;
//            putsU2("CMD"); 
        }
  
    }
    
    //RESP = SD_CMD(CMD16,0x00,0x00,0x02,0x00,0xFF);          //CMD16 set block size to 512
    
    SD_CARD.init_code = 0;
    return &SD_CARD;
    //return 0x00;                //init successful
}

unsigned char SD_CMD(unsigned char CMD, unsigned char ARG1, unsigned char ARG2, unsigned char ARG3, unsigned char ARG4, unsigned char CRC)
{
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    SD_CS = 0;
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    
    SPI_write_byte(CMD);
    SPI_write_byte(ARG1);
    SPI_write_byte(ARG2);
    SPI_write_byte(ARG3);
    SPI_write_byte(ARG4);
    SPI_write_byte(CRC);
    
    unsigned char i=0;
    TMR3 = 0;
    while(TMR3 < 4000);
    while(i<8)
    {
        RES_BUF[i] = SPI_read_byte();     //response byte
        i++;
    }
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    
    SD_CS = 1;
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    return RES_BUF[1];     //response byte
}


unsigned char SD_Read_Block(unsigned char * buffer, unsigned long ADDR)
{
    unsigned int k;
    k = 0;
    ARG[0] = (unsigned char)(ADDR >> 24);
    ARG[1] = ADDR >> 16;
    ARG[2] = ADDR >> 8;
    ARG[3] = ADDR;
    
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    
    SD_CS = 0;
    TEMP = SPI_read_byte();         //prep the SD cad with some clock
    SPI_write_byte(0x51);          //0x51 = CMD17
    SPI_write_byte(ARG[0]);
    SPI_write_byte(ARG[1]);
    SPI_write_byte(ARG[2]);
    SPI_write_byte(ARG[3]);
    SPI_write_byte(0xFF);
    //TEMP = SPI_read_byte();         //prep the SD cad with some clock
    while(1)                //looking for command ACK 0x00
    {
        TEMP = SPI_read_byte();
        if(TEMP==0x00)
        {
            asm("nop");
            break;
        }
        else
        {
        asm("nop");
        asm("nop");
        asm("nop");
        }
    }
    
    while(1)            //looking for 0xFE start of block
    {    
        TEMP = SPI_read_byte();
        if(TEMP == 0xFE)
        {
            asm("nop");
            break;
        }
        else
        {
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
        }
    }
    
    while(k<512)
    {
        //SD_BUF[k] = SPI_read_byte();
        *(buffer+k) = SPI_read_byte();
        k++;
    }
    SD_CS = 1;
    return 0;
}