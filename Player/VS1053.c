#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <p24EP512GU810.h>
#include "VS1053.h"
#include "SPI.h"

enum AudioFormat {
  afUnknown,
  afRiff,
  afOggVorbis,
  afMp1,
  afMp2,
  afMp3,
  afAacMp4,
  afAacAdts,
  afAacAdif,
  afFlac,
  afWma,
  afMidi,
} audioFormat = afUnknown;

const char *afName[] = {
  "unknown",
  "RIFF",
  "Ogg",
  "MP1",
  "MP2",
  "MP3",
  "AAC MP4",
  "AAC ADTS",
  "AAC ADIF",
  "FLAC",
  "WMA",
  "MIDI",
};





/*
  Read 32-bit increasing counter value from addr.
  Because the 32-bit value can change while reading it,
  read MSB's twice and decide which is the correct one.
*/
u_int32 ReadVS10xxMem32Counter(u_int16 addr) {
  u_int16 msbV1, lsb, msbV2;
  u_int32 res;

  WriteSci(SCI_WRAMADDR, addr+1);
  msbV1 = ReadSci(SCI_WRAM);
  WriteSci(SCI_WRAMADDR, addr);
  lsb = ReadSci(SCI_WRAM);
  msbV2 = ReadSci(SCI_WRAM);
  if (lsb < 0x8000U) {
    msbV1 = msbV2;
  }
  res = ((u_int32)msbV1 << 16) | lsb;
  
  return res;
}


/*
  Read 32-bit non-changing value from addr.
*/
u_int32 ReadVS10xxMem32(u_int16 addr) {
  u_int16 lsb;
  WriteSci(SCI_WRAMADDR, addr);
  lsb = ReadSci(SCI_WRAM);
  return lsb | ((u_int32)ReadSci(SCI_WRAM) << 16);
}


/*
  Read 16-bit value from addr.
*/
u_int16 ReadVS10xxMem(u_int16 addr) {
  WriteSci(SCI_WRAMADDR, addr);
  return ReadSci(SCI_WRAM);
}


/*
  Write 16-bit value to given VS10xx address
*/
void WriteVS10xxMem(u_int16 addr, u_int16 data) {
  WriteSci(SCI_WRAMADDR, addr);
  WriteSci(SCI_WRAM, data);
}

/*
  Write 32-bit value to given VS10xx address
*/
void WriteVS10xxMem32(u_int16 addr, u_int32 data) {
  WriteSci(SCI_WRAMADDR, addr);
  WriteSci(SCI_WRAM, (u_int16)data);
  WriteSci(SCI_WRAM, (u_int16)(data>>16));
}




static const u_int16 linToDBTab[5] = {36781, 41285, 46341, 52016, 58386};

/*
  Converts a linear 16-bit value between 0..65535 to decibels.
    Reference level: 32768 = 96dB (largest VS1053b number is 32767 = 95dB).
  Bugs:
    - For the input of 0, 0 dB is returned, because minus infinity cannot
      be represented with integers.
    - Assumes a ratio of 2 is 6 dB, when it actually is approx. 6.02 dB.
*/
static u_int16 LinToDB(unsigned short n) {
  int res = 96, i;

  if (!n)               /* No signal should return minus infinity */
    return 0;

  while (n < 32768U) {  /* Amplify weak signals */
    res -= 6;
    n <<= 1;
  }

  for (i=0; i<5; i++)   /* Find exact scale */
    if (n >= linToDBTab[i])
      res++;

  return res;
}




/*

  Loads a plugin.

  This is a slight modification of the LoadUserCode() example
  provided in many of VLSI Solution's program packages.

*/
void LoadPlugin(const u_int16 *d, u_int16 len) {
  int i = 0;

  while (i<len) {
    unsigned short addr, n, val;
    addr = d[i++];
    n = d[i++];
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = d[i++];
      while (n--) {
        WriteSci(addr, val);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = d[i++];
        WriteSci(addr, val);
      }
    }
  }
}





u_int8 adpcmHeader[60] = {
  'R', 'I', 'F', 'F',
  0xFF, 0xFF, 0xFF, 0xFF,
  'W', 'A', 'V', 'E',
  'f', 'm', 't', ' ',
  0x14, 0, 0, 0,          /* 20 */
  0x11, 0,                /* IMA ADPCM */
  0x1, 0,                 /* chan */
  0x0, 0x0, 0x0, 0x0,     /* sampleRate */
  0x0, 0x0, 0x0, 0x0,     /* byteRate */
  0, 1,                   /* blockAlign */
  4, 0,                   /* bitsPerSample */
  2, 0,                   /* byteExtraData */
  0xf9, 0x1,              /* samplesPerBlock = 505 */
  'f', 'a', 'c', 't',     /* subChunk2Id */
  0x4, 0, 0, 0,           /* subChunk2Size */
  0xFF, 0xFF, 0xFF, 0xFF, /* numOfSamples */
  'd', 'a', 't', 'a',
  0xFF, 0xFF, 0xFF, 0xFF
};

u_int8 pcmHeader[44] = {
  'R', 'I', 'F', 'F',
  0xFF, 0xFF, 0xFF, 0xFF,
  'W', 'A', 'V', 'E',
  'f', 'm', 't', ' ',
  0x10, 0, 0, 0,          /* 16 */
  0x1, 0,                 /* PCM */
  0x1, 0,                 /* chan */
  0x0, 0x0, 0x0, 0x0,     /* sampleRate */
  0x0, 0x0, 0x0, 0x0,     /* byteRate */
  2, 0,                   /* blockAlign */
  0x10, 0,                /* bitsPerSample */
  'd', 'a', 't', 'a',
  0xFF, 0xFF, 0xFF, 0xFF
};

void Set32(u_int8 *d, u_int32 n) {
  int i;
  for (i=0; i<4; i++) {
    *d++ = (u_int8)n;
    n >>= 8;
  }
}

void Set16(u_int8 *d, u_int16 n) {
  int i;
  for (i=0; i<2; i++) {
    *d++ = (u_int8)n;
    n >>= 8;
  }
}



/*

  Hardware Initialization for VS1053.

  
*/
int VSTestInitHardware(void) {
  /* Write here your microcontroller code which puts VS10xx in hardware
     reset anc back (set xRESET to 0 for at least a few clock cycles,
     then to 1). */
    TRISAbits.TRISA7 = 0;           //TRIS for XCS set to output
    TRISGbits.TRISG12 = 0;          //TRIS for XDCS set to output
    ANSELAbits.ANSA7 = 0;           //analog mode
    
    TRISAbits.TRISA5 = 0;           //XRESET
    TRISAbits.TRISA4 = 1;           //DREQ set to input
    
    
    XCS = 1;
    XDCS = 1;
    XRESET = 0;
    TMR3 = 0;                       //perform hardware reset
    while(TMR3 < 10000);
    XRESET = 1;
    while(DREQ);                    //ensure DREQ go low
    while(!DREQ);                   //ensure DREQ go high
    TMR3 = 0;
    while(TMR3 < 50000);             //require to wait atleast 1.8ms for DREQ after reset
    return 0;
}



/* Note: code SS_VER=2 is used for both VS1002 and VS1011e */
const u_int16 chipNumber[16] = {
  1001, 1011, 1011, 1003, 1053, 1033, 1063, 1103,
  0, 0, 0, 0, 0, 0, 0, 0
};

/*

  Software Initialization for VS1053.

  Note that you need to check whether SM_SDISHARE should be set in
  your application or not.
  
*/
int VSTestInitSoftware(void) {
    u_int16 ssVer;

  /* Start initialization with a dummy read, which makes sure our
     microcontoller chips selects and everything are where they
     are supposed to be and that VS10xx's SCI bus is in a known state. */
    while(!DREQ);                   //ensure DREQ is not low
    ssVer = ReadSci(SCI_MODE);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");

//  WriteSci(SCI_MODE,SM_RESET);
//  TMR3 = 0;
//  while(TMR3 < 10000);
    while(!DREQ);                   //ensure DREQ is not low
    ssVer = ReadSci(SCI_STATUS);
    asm("nop");
    asm("nop");
    //WriteSci(SCI_MODE, SM_SDINEW|SM_TESTS);
    WriteSci(SCI_MODE, SM_SDINEW);
    
    asm("nop");
    asm("nop");
    TMR3 = 0;
    while(TMR3 < 1000);
    ssVer = ReadSci(SCI_MODE);
    asm("nop");
    asm("nop");

    
    asm("nop");
    asm("nop");
  //WriteSci(SCI_MODE, SM_SDINEW|SM_TESTS|SM_RESET);          //no SM_SDISHARE

  /* A quick sanity check: write to two registers, then test if we
     get the same results. Note that if you use a too high SPI
     speed, the MSB is the most likely to fail when read again. */
    WriteSci(SCI_AICTRL1, 0xABAD);
    WriteSci(SCI_AICTRL2, 0x7E57);
    u_int16 val1, val2;
    val1 = ReadSci(SCI_AICTRL1);
    val2 = ReadSci(SCI_AICTRL2);
    asm("nop");
    asm("nop");
    if (ReadSci(SCI_AICTRL1) != 0xABAD || ReadSci(SCI_AICTRL2) != 0x7E57) {
      printf("There is something wrong with VS10xx SCI registers\n");
      return 1;
    }
    WriteSci(SCI_AICTRL1, 0);
    WriteSci(SCI_AICTRL2, 0);

    /* Check VS10xx type */
    ssVer = ((ReadSci(SCI_STATUS) >> 4) & 15);
    if (chipNumber[ssVer]) {
      printf("Chip is VS%d\n", chipNumber[ssVer]);
      if (chipNumber[ssVer] != 1053) {
        printf("Incorrect chip\n");
        return 1;
      }
    } else {
      printf("Unknown VS10xx SCI_MODE field SS_VER = %d\n", ssVer);
      return 1;
    }

    /* Set the clock PLL = 12.88MHz * (3.5 + 1) = 55.3MHz
     * Max SCI Read = CLKI/7 = 55.3MHz/7 = 7.9Mhz*/
    WriteSci(SCI_CLOCKF, SC_MULT_53_35X | SC_ADD_53_10X); //

    /* Set up other parameters. */
    //WriteVS10xxMem(PAR_CONFIG1, PAR_CONFIG1_AAC_SBR_SELECTIVE_UPSAMPLE);

    /* Set volume level at -6 dB of maximum */
    WriteSci(SCI_VOL, 0x0F0F);
    return 0;
}






void WriteSci(u_int8 addr, u_int16 data)
{
    unsigned char temp;
    XDCS = 1;
    XCS = 0;
    SPI_write_byte(SCI_WRITE);
    SPI_write_byte(addr);
    temp = (data >> 8) & 0xFF;
    SPI_write_byte(temp);       //write MSB
    temp = (data & 0xFF);
    SPI_write_byte(temp);            //write LSB
    XCS = 1;
    while(!DREQ);
}

u_int16 ReadSci(u_int8 addr)
{
    u_int16 rtnVAL = 0;
    unsigned char temp1, temp2;
    XDCS = 1;
    XCS = 0;
    SPI_write_byte(SCI_READ);
    SPI_write_byte(addr);
    temp1 = SPI_read_byte();
    temp2 = SPI_read_byte();
    rtnVAL = temp1 & 0xFF;
    rtnVAL = (rtnVAL << 8) | temp2;
    XCS = 1;
    while(!DREQ);
    return rtnVAL;
}


int WriteSdi(const u_int8 *data, u_int8 bytes)
{
    XDCS = 0;
    u_int8 i;
    for(i = 0; i<bytes; i++)
    {
        SPI_write_byte(*(data+i));
    }
    XDCS = 1;
    return 0;
}


void testMP3Sine(u_int8 n)
{
    // Send a Sine Test Header to Data port
    XDCS = 0;               // enable data interface
    SPI_write_byte( 0x53);                // special Sine Test Sequence
    SPI_write_byte( 0xef);		
    SPI_write_byte( 0x6e);
    SPI_write_byte( n);                   // n, Fsin = Fsamp[n>>5] * (n & 0x1f) / 128
    SPI_write_byte( 0x00);                // where Fsamp[x] = {44100,48000,32000,22050,
    SPI_write_byte( 0x00);                //                   24000,16000,11025,12000}
    SPI_write_byte( 0x00);                // for example n = 0x44 -> 1KHz
    SPI_write_byte( 0x00);		
    XDCS = 1;		

//    DelayMs( 500);
//
//    // Stop the sine test
//    XDCS = 0;               // enable data interface
//    SPI_write_byte( 0x45);                // special Sine Test termination sequence
//    SPI_write_byte( 0x78);		
//    SPI_write_byte( 0x69);		
//    SPI_write_byte( 0x74);		
//    SPI_write_byte( 0x00);		
//    SPI_write_byte( 0x00);		
//    SPI_write_byte( 0x00);		
//    SPI_write_byte( 0x00);		
//    XDCS = 1;		
//
//    DelayMs( 500);
} // SineTest
