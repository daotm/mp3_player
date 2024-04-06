/*

  VLSI Solution generic microcontroller example player / recorder definitions.
  v1.00.

  See VS10xx AppNote: Playback and Recording for details.

  v1.00 2012-11-23 HH  First release

*/
#ifndef VS1053_H
#define VS1053_H

#include "vs10xx_uc.h"

#define SCI_READ    0x03            //0b0000 0011 READ OP CODE
#define SCI_WRITE   0x02            //0b0000 0010 WRITE OP CODE
#define XCS         LATAbits.LATA7
#define XDCS        LATGbits.LATG12
#define XRESET      LATAbits.LATA5
#define DREQ        PORTAbits.RA4

#define FILE_BUFFER_SIZE 512
#define SDI_MAX_TRANSFER_SIZE 32
#define SDI_END_FILL_BYTES_FLAC 12288
#define SDI_END_FILL_BYTES       2050
#define REC_BUFFER_SIZE 512
#define REPORT_INTERVAL 4096
#define REPORT_INTERVAL_MIDI 512

#define min(a,b) (((a)<(b))?(a):(b))





int VSTestInitHardware(void);
int VSTestInitSoftware(void);


void WriteSci(u_int8 addr, u_int16 data);
u_int16 ReadSci(u_int8 addr);
int WriteSdi(const u_int8 *data, u_int8 bytes);
void testMP3Sine(u_int8 n);

#endif
