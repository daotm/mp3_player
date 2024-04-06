
#ifndef PLAYER_RECORDER_H
#define PLAYER_RECORDER_H

#include "SPI.h"
#include "SD.h"
#include "VS1053.h"

typedef struct RECORD
{
    unsigned char name[8];
    unsigned long FIRST_CLUSTER;
}RECORD;




RECORD * init_playlist(SD_INFO *);


unsigned int get_playlist_sz();

int play_song(RECORD * song);
unsigned long combine_ADDR(unsigned char LSB, unsigned char LB, unsigned char HB, unsigned char MSB);
int play_cluster(unsigned long cluster_offset);
unsigned long find_next_cluster(unsigned long curr_cluster_addr);

void play_fill_byte();

void testMP3Sine(u_int8 n);

#endif
