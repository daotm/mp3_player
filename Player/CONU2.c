
#include "CONU2.h"
#include "xc.h"


void InitU2( void)
{
    _U2RXR = 0x43;
    _RP65R = 0x03;
    U2BRG = BRATE;
    U2MODE = U_ENABLE;
    U2STA = U_TX;
    RTS = 1; // set RTS default status
    TRTS = 0; // make RTS output
} // InitU2

int putU2( int c)
{
    while ( CTS); // wait for !CTS, clear to send
    while ( U2STAbits.UTXBF); // wait while Tx buffer full
    U2TXREG = c;
    return c;
} // putU2

char getU2( void)
{
    RTS = 0; // assert Request To Send !RTS
    while ( !U2STAbits.URXDA); // wait
    RTS = 1;
    return U2RXREG; // read from the receive buffer
}// getU2

void putsU2( char *s)
{
    while( *s) // loop until *s == '\0' the end of the string
    { 
        putU2( *s++); // send the character and point to the next one
    }
    putU2( '\r'); // terminate with a cr / line feed
    putU2( '\n');
} // putsU2

char *getsnU2( char *s, int len)
{
    char *p = s; // copy the buffer pointer
    do{
        *s = getU2(); // wait for a new character
        putU2( *s); // echo character
        if (( *s==BACKSPACE)&&( s>p))
        {
            putU2( ' '); // overwrite the last character
            putU2( BACKSPACE);
            len++;
            s--; // back the pointer
            continue;
        }
        if ( *s=='\n') // line feed, ignore it
            continue;
        if ( *s=='\r') // end of line, end loop
            break;
        s++; // increment buffer pointer
        len--;
    } while ( len>1 ); // until buffer full
    *s = '\0'; // null terminate the string
    return p; // return buffer pointer
} // getsnU2