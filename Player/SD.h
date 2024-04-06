#ifndef SD_H
#define SD_H


#include "xc.h"
#include <p24EP512GU810.h>
#include "SPI.h"
#include "CONU2.h"


#define SD_CS       LATGbits.LATG9
#define CMD0        0x40
#define CMD8        0x48
#define CMD17       0x51
#define CMD16       0x50


typedef struct SD_INFO
{
    unsigned char init_code;                //initialization return code
    unsigned long BPB_OFFSET;
    unsigned long SECTOR_SZ;
    unsigned long CLUSTER_SZ;
    unsigned long FAT_SZ;
    unsigned long RESERVED_SZ;
    unsigned long FAT_NUM;                   //number of fat table
    unsigned long FIRST_DAT_SECTOR;         //info directory
    
}SD_INFO;



SD_INFO * SD_init();
unsigned char SD_CMD(unsigned char CMD, unsigned char ARG1, unsigned char ARG2, unsigned char ARG3, unsigned char ARG4, unsigned char CRC);
unsigned char SD_Read_Block(unsigned char * buffer, unsigned long ADDR);

#endif