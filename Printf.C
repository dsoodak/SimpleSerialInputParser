#include <stdarg.h>	 //va_list 	 va_start()    va_arg()

#include "SerialIO.h"//or just define PutChar()

//Dustin Soodak
//dsoodak yahoo mail (put "github" in subject)

#define Def_Mode 0
#define Percent_Sign_Found 1
#define First_Fill_Digit_Found 2
#define Check_If_Print_Mode_Character 3
#define Character_Mode 4
#define String_Mode 5
#define Unsigned_Integer_Mode 6
#define Hex_Upper_Case_Mode 7
#define Hex_Lower_Case_Mode 8
#define Binary_Mode 9
#define Integer_Mode 10
#define Print_Mode 11
#define Float_Mode 12
#define First_DecimalPlace_Digit_Found 13

#define DecToInt(x) ((x)-'0')
#define HexToInt(x) ((x)<='9'?(x)-'0':(x)<='F'?(x)-'A'+10:(x)-'a'+10)
#define HexToIntLowerCase(x) ((x)<='9'?(x)-'0':(x)-'a'+10)
#define HexToIntUpperCase(x) ((x)<='9'?(x)-'0':(x)-'A'+10)
#define IntToHexLowerCase(x) ((x)<10?'0'+(x):'a'+(x)-10)
#define IntToHexUpperCase(x) ((x)<10?'0'+(x):'A'+(x)-10)



char printf_printing=0;
int PrintfCollisions=0;//for when running on a chip with multiple interrupts that use the same serial port

//Dustin Soodak
void Printf(const char *pcFullString, ...)
{
    unsigned long long Value;//64 bits so it can take longer floats.
    double ValueF;
    int i;
    char FillCharacter='0',FillLength=0,DecimalPlaces=0,Base=2,IsNegative,Mode;
    int StringLength=0;
    char *strValue=0;
    char buffer[20];
    char EscChar=0;
    const char *pcString=pcFullString;
    va_list vaArg;
    va_start(vaArg, pcFullString);
    Mode=Def_Mode;
    if(!printf_printing){
        printf_printing=1;
        while(*pcString!=0){
            switch(Mode){
                case Def_Mode://printing string up until a '%' is found
                    if(EscChar){
                        EscChar=0;
                        PutChar(*pcString);
                    }
                    else{
                        if(*pcString=='%'){
                            Mode=Percent_Sign_Found;
                            FillCharacter=' ';//if there is a number before the type specifier character, and it starts with a 0, this is changed to '0'
                            FillLength=0;
                            DecimalPlaces=0;
                        }
                        else{
                            if(*pcString=='\\')
                                EscChar=1;
                            else
                                PutChar(*pcString);
                        }
                    }
                break;
                case Percent_Sign_Found:
                    if('0'<=*pcString && *pcString<='9'){
                        if(*pcString=='0') FillCharacter='0';
                        else FillLength=DecToInt(*pcString);
                        Mode=First_Fill_Digit_Found;
                    }
                    else if(*pcString=='.'){
                        Mode=First_DecimalPlace_Digit_Found;
                        DecimalPlaces=0;
                    }
                    else{
                        Mode=Check_If_Print_Mode_Character;
                    }
                break;
                case First_Fill_Digit_Found:
                    if('0'<=*pcString && *pcString<='9'){
                        FillLength*=10;
                        FillLength+=DecToInt(*pcString);
                    }
                    else if(*pcString=='.'){
                        Mode=First_DecimalPlace_Digit_Found;
                        DecimalPlaces=0;
                    }
                    else{
                        Mode=Check_If_Print_Mode_Character;
                    }
                break;
                case First_DecimalPlace_Digit_Found: //though actually, only the "." between fill and decimal place digits was found
                    if('0'<=*pcString && *pcString<='9'){
                        DecimalPlaces*=10;
                        DecimalPlaces+=DecToInt(*pcString);
                    }
                    else{
                        Mode=Check_If_Print_Mode_Character;//should be an 'f' since decimal binary & hex not currently supported
                    }

                break;
                case Check_If_Print_Mode_Character:
                    switch(*pcString){
                        case 'c':
                            Mode=Character_Mode;
                        break;
                        case 's':
                            Mode=String_Mode;
                        break;
                        case 'd':	//"*(unsigned int*)&" put in front of 64 bit number (long long or uint64_t) or "va_arg(vaArg, unsigned int)" may first input a zero value before reading the first numerical input.
                            Mode=Integer_Mode;Base=10;
                        break;
                        case 'f':					//%6.4f -> min size 6 with 3 decimal places.
                            Mode=Float_Mode;Base=10;
                        break;
                        case 'u':
                            Mode=Unsigned_Integer_Mode;Base=10;
                        break;
                        case 'X':
                            Mode=Hex_Upper_Case_Mode;
                            Base=16;
                        break;
                        case 'x':
                            Mode=Hex_Lower_Case_Mode;
                            Base=16;
                        break;
                        case 'b':
                            Mode=Binary_Mode;
                            Base=2;
                        break;
                        case '%':
                            PutChar(*pcString);//print '%'
                            Mode=Def_Mode;
                        break;
                        default://not a valid print mode character.
                            Mode=Def_Mode;
                        break;
                    }
                break;
                case Print_Mode:
                    for(i=0;i<FillLength-StringLength;i++)PutChar(FillCharacter);
                    for(i=0;i<StringLength;i++)PutChar(strValue[i]);
                    Mode=Def_Mode;
                break;
                case Character_Mode:
                    Value = va_arg(vaArg, unsigned int);
                    if(Value>0){
                        StringLength=1;
                        strValue=(char*)&Value;
                    }
                    else
                        StringLength=0;
                    if(FillCharacter=='0')
                        FillCharacter='_';
                    Mode=Print_Mode;
                break;
                case String_Mode:
                    strValue = va_arg(vaArg, char*);
                    StringLength=0;
                    while(strValue[StringLength]>0)
                        StringLength=StringLength+1;
                    if(FillCharacter=='0')
                        FillCharacter='_';
                    Mode=Print_Mode;
                break;
                case Float_Mode:
                case Unsigned_Integer_Mode:
                case Integer_Mode:
                case Hex_Lower_Case_Mode:
                case Hex_Upper_Case_Mode:
                case Binary_Mode:
                    if(DecimalPlaces>0)
                        ValueF = va_arg(vaArg, double);//compiler converts float to double in variable argument functions
                    else
                        Value = va_arg(vaArg, unsigned int);//used for cases besides float
                    //if signed and negative, make positive (unless float: then do what is in new handler below)
                    if(DecimalPlaces>0){ //'%f'
                        //add one extra virtual decimal place(so we can round)
                        for(i=0;i<DecimalPlaces+1;i++)
                            ValueF*=Base;
                        //make positive
                        if(ValueF<0.){
                            IsNegative=1;
                            Value=(-ValueF);
                        }
                        else{
                            IsNegative=0;
                            Value=ValueF;
                        }
                        //round the number
                        if((Value%Base)<(Base/2+(Base&1)))//if Base=10, <5, if base=7, <4
                            Value=(Value/Base);
                        else
                            Value=(Value/Base)+1;
                        //ex: -1.2345 with 3 decimal places -> Value=1235;IsNegative=1;
                        //and will be printed as "1.235"

                    }
                    else if(Mode==Integer_Mode && ((signed int)Value)<0){
                        Value=-((signed int)Value);
                        IsNegative=1;
                    }
                    else{
                        IsNegative=0;
                    }
                    i=sizeof(buffer)-1;
                    while(Value){
                        //add decimal point if and when appropriate
                        //This section will not appear in above section  "Mode==Hex_Upper_Case_Mode"
                        if(sizeof(buffer)-1-i==DecimalPlaces && DecimalPlaces>0){ //only called once in loop
                            buffer[i]='.';
                            i--;
                        }
                        if(i>=0){
                            if(Mode==Hex_Upper_Case_Mode)
                                buffer[i]=IntToHexUpperCase(Value%Base);
                            else
                                buffer[i]=IntToHexLowerCase(Value%Base);
                            Value/=Base;
                            i--;
                        }
                    }
                    while(sizeof(buffer)-1-i<=DecimalPlaces){//if there should be a decimal point but not yet printed, this will print it after addign zeroes if necessary.
                        if(sizeof(buffer)-1-i==DecimalPlaces && DecimalPlaces>0){	//only called once in loop
                            buffer[i]='.';
                            i--;
                        }
                        if(i>=0){				//should fill in zeroes after decimal point (printed first) and one before decimal point (printed just before exiting loop)
                            buffer[i]='0';
                            i--;
                        }
                    }
                    if(IsNegative){
                        buffer[i]='-';
                        i--;
                    }
                    if(i==sizeof(buffer)-1){//if Value was initially ==0
                        buffer[i]='0';
                        i--;
                    }
                    StringLength=sizeof(buffer)-1-i;
                    strValue=&buffer[i+1];
                    Mode=Print_Mode;
                break;

            }//end switch(Mode)
            if(Mode==Def_Mode || Mode==Percent_Sign_Found || Mode==First_Fill_Digit_Found || Mode==First_DecimalPlace_Digit_Found)
                pcString++;
        }//end while(*pcString)

        va_end(vaArg);
    printf_printing=0;
    }//end if(!printf_printing)
    else
        PrintfCollisions++;
}//end void Printf(const char *String, ...)
