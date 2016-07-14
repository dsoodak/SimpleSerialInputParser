#ifndef SERIAL_IO_H
#define SERIAL_IO_H

//Dustin Soodak
//dsoodak google mail



    typedef char CharType;

    /*
    //A struct of other serial data functions I have found reason to create over the years
    typedef struct{
        int (*Init)(params);//params could be anything or nothing depending on the hardware
        int (*RxAvail)(void);//must at least be able to return 0 (none) or 1 (one or more)
        CharType (*Rx)(void);//waits till one character arrives
        int (*RxNonBlocking)(void);//returns int equivalent to character, or -1 if no characters immediately available from Rx buffer
        int (RxPeek)(void);//same as RxNonBlocking except that it doesn't remove the character from the buffer after reading.
        int (*TxAvail);//how much free space in the transmit buffer
        void (*TxChar)(CharType Send);
        int (*TxCharNonBlocking)(CharType Send);//for cases where there is a buffer, or a shared bus
        void (*TxString)(CharType *Send);//useful for network communication or if you want to send more than one character every few ms over serial usb.
        int (*TxStringNonBlocking)(CharType *Send);
    } SerialType;*/


    void InitSerial(void);
    int CharsAvail(void);//must at least be able to return 0 (none) or 1 (one or more)
    void PutChar(CharType Character);
    int PrintString(CharType *Str, int Length);
    int GetCharNonBlocking(void);//should return -1 if no character available
    CharType GetChar(void);




/*
#include <iostream>
using namespace std;
//both of these are for cout, endl
#include <conio.h> //in windows, putch(), kbhit()
int main()
{
    char c;
    while(c!='x'){
        if (kbhit()!=0) {
            c=getch();
            cout<<c<<endl;
        }
    }
}
*/



#endif

