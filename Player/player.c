#include "player.h"

unsigned char SD_buff[512];
unsigned char FAT_buff[512];
unsigned long FAT_loc;                          //location trakcer for the lcoation in FAT table.
unsigned long FAT_offset;

RECORD playlist[256];
unsigned int PLAYLIST_CNT;

SD_INFO * SD_PTR;


unsigned int get_playlist_sz()
{
    return PLAYLIST_CNT;
}


RECORD * init_playlist(SD_INFO * PTR)
{
    unsigned int i, j, k;
    unsigned int ext_offset = 0;
    unsigned int num_sector_to_check = 20;
    unsigned int file_per_sect;
    
    SD_PTR = PTR; 
    PLAYLIST_CNT = 0;
    
    
    file_per_sect = SD_PTR->SECTOR_SZ/32;           //32 bytes allocated to each file.
    
    //scan the first 20 sector starting the first data sector to look for MP3 extension
    for(i=0; i < num_sector_to_check; i++)
    {
        SD_Read_Block(&SD_buff[0], SD_PTR->FIRST_DAT_SECTOR + i);
        for(j=0; j < file_per_sect; j++)
        {
            ext_offset = (j*32)+8;
            if(SD_buff[ext_offset] == 'M' && SD_buff[ext_offset+1] == 'P' && SD_buff[ext_offset+2] == '3')
            {
                for(k=0; k < 8; k++)
                    playlist[PLAYLIST_CNT].name[k] = SD_buff[ext_offset-8 + k];   
                playlist[PLAYLIST_CNT].FIRST_CLUSTER = combine_ADDR(SD_buff[ext_offset-8+26], 
                        SD_buff[ext_offset-8+27], SD_buff[ext_offset-8+20], SD_buff[ext_offset-8+21]);
                PLAYLIST_CNT++;
            }
        } 
    }
    asm("nop");
    asm("nop");
    asm("nop");
    return &playlist[0];
}




int play_song(RECORD * song)
{
    unsigned long curr_cluster = song->FIRST_CLUSTER;
    unsigned long next_cluster = 0;
    FAT_loc = 0;
    FAT_offset = 0;
    SD_Read_Block(&FAT_buff[0], 2080);                  //reserved sector + BPB_offset = 32 + 2048 =  
    
    while(1)
    {
        play_cluster(curr_cluster);
        next_cluster = find_next_cluster(curr_cluster);
        if(next_cluster == 0x0FFFFFFF)
            break;
        curr_cluster = next_cluster;
    }
    
    //write endfill byte of 2048 byte of 0 to make sure song finish playing
    play_fill_byte();
    return 0;
}


int play_cluster(unsigned long cluster_offset)
{
    unsigned int i, j;
    unsigned int chunk, byte_offset;
    unsigned char * fp;
    
    unsigned long sect_offset;
    sect_offset = (cluster_offset-2)*(SD_PTR->CLUSTER_SZ);
    sect_offset = sect_offset + SD_PTR->FIRST_DAT_SECTOR; 
           
    
    for(i=0; i < SD_PTR->CLUSTER_SZ; i++)               //cluster size of 32 sectors
    {
        SD_Read_Block(&SD_buff[0], sect_offset+i);
        fp = &SD_buff[0];
        chunk = 0;
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        while(chunk < 16)              //send 16 chunks of data at a time
        {
            byte_offset = chunk * 32;
            while(!DREQ);
            XDCS = 0;               //enable MP3 data bus
            for(j = 0; j < 32; j++)
            {
                SPI_write_byte(*(fp+byte_offset+j));
            }
            XDCS = 1;               //disable data bus after every 32 bytes sent
            chunk++;
        }
        
    }
    return 0;
}



unsigned long find_next_cluster(unsigned long curr_cluster_addr)
{
    //check if the current location is shown in the FAT
    unsigned int offset_loc;
    unsigned long next_cluster_addr;
    
    // Need to update FAT Loc prior to start

    offset_loc = curr_cluster_addr & 0x7F;                    //lower nibble is the offset inside each FAT sector;
    if(offset_loc >= 127)
        asm("nop");
    if(curr_cluster_addr >= ((FAT_offset+1)*128))                    //each FAT sector contains 16 address//great or = mean it's not the correct sector;
    {   
        FAT_offset = curr_cluster_addr >> 7;                     //right shift 7 = /128 to find the FAT offset
        FAT_loc = SD_PTR->BPB_OFFSET + SD_PTR->RESERVED_SZ + FAT_offset;          //BPB offset + reserved + FAT offset;
        SD_Read_Block(&FAT_buff[0], FAT_loc);                   //FAT loc is the current sector offset
    }
    offset_loc = offset_loc*4;
    next_cluster_addr = combine_ADDR(FAT_buff[offset_loc], FAT_buff[offset_loc+1], FAT_buff[offset_loc+2], FAT_buff[offset_loc+3]);
    return next_cluster_addr;           
}


unsigned long combine_ADDR(unsigned char LSB, unsigned char LB, unsigned char HB, unsigned char MSB)
{
    unsigned long combine_val = LSB & 0xFF;
    combine_val = combine_val | ((unsigned long) LB << 8);
    combine_val = combine_val | ((unsigned long) HB << 16) ;
    combine_val = combine_val | ((unsigned long) MSB << 24);
    return combine_val;
}





void play_fill_byte()
{
    unsigned char i,j;
    for(i=0; i<16; i++)
    {
        while(!DREQ);
        XDCS = 0;               //enable MP3 data bus
        for(j = 0; j < 32; j++)
        {
            SPI_write_byte(0x00);
        }
        XDCS = 1;               //disable data bus after every 32 bytes sent
    }
}