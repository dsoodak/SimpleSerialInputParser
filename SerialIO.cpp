#include "SerialIO.h"

#include <stdio.h> //for fflush
//This file changed from .c to .cpp so it would be possible to
//include this library.

//Dustin Soodak
//dsoodak yahoo mail


    void InitSerial(void){
        fflush(stdin);
    }

    #include <conio.h> //in windows, putch(), kbhit()
    void PutChar(CharType Character){
            putch(Character);
    }

    //If something like usb serial that sends data in packets, this will work faster if you
    //don't just implement it by calling PutChar() a bunch of times in a row.
    int PrintString(CharType *Str, int Length){
        int i=0;
        if(Length){
            for(i=0;i<Length;i++){
                PutChar(Str[i]);
            }
        }
        else{
            while(Str[i]){
                PutChar(Str[i]);
                i++;
            }
        }
        return i;
    }

    int CharsAvail(void){
        return kbhit();//SerialRxCharactersAvailable();
    }

    int GetCharNonBlocking(void){
        char c;
        if (kbhit()!=0){
            c=getch();
            if(c=='\r')
                c='\n';
            return c;
        }
        else
            return -1;
    }
    CharType GetChar(void){
        CharType c=getch();
        //In this case (gcc/dos), \r is received but \n is always skipped
        //However, terminal doesn't seem to mind a \n without a \r (still does regular newline)
        if(c=='\r')
            c='\n';
        return c;
    }


    //Add something like the following to the GUI code:




