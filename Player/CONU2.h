/*
** CONU2.h
** console I/O library for Explorer16 board
*/
// I/O definitions for the Explorer16
#define CTS _RF8 // Clear To Send, in, handshake
#define RTS _RF13 // Request To Send, out, handshake
#define TRTS TRISFbits.TRISF13 // tris control for RTS pin
#define BACKSPACE 0x8 // ASCII backspace character code
#define BRATE 8  // 115200 Baud (BREGH=1)
#define U_ENABLE 0x8008 // enable the UART peripheral
#define U_TX     0x0400 //enable transmission

// useful macros
#define Clrscr() putsU2( "\x1b[2J") // Clear the screen
#define Home() putsU2( "\x1b[1;1H") // return cursor home
#define pcr() putU2( '\r' ); putU2( '\n') // carriage return

// init the serial port (UART2, 115200, 8, N, 1, CTS/RTS)
void InitU2( void);
// send a character to the serial port
int putU2( int c);
// wait for a new character to arrive to the serial port
char getU2( void);
// send a null terminated string to the serial port
void putsU2( char *s);
// receive a null terminated string in a buffer of len char
char * getsnU2( char *s, int n);