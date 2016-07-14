
#include "SerialInputParser.h"

#include "SerialIO.h" //If not windows CLI, just define your own versions of PutChar()
                      //and GetCharNonBlocking() (which returns -1 if character buffer is empty).

#include "Printf.h" //I'm using my printf fuction, but you can also just #include <cstdio>
#define printf Printf


//Dustin Soodak
//dsoodak yahoo mail (put "github" in subject)


//The main tokenizing function is TokenizeInput().
//This can be used to make specialized user-friendly functions such as TokenizeSerialInput() and TokenizeStringInput().
//
//The tokenizing functions use a pre-allocated buffer to store the list of tokens and other information related to the process.
//#define MAX_NUM_OF_INPUT_CHARACTERS 256
//char SerialInput_Buffer[MAX_NUM_OF_INPUT_CHARACTERS];
//
//This buffer must be initialized once:
//char **ArgV;
//const char *PredefTokenList[]={"++","+","-",""};
//TokenizeInit(&ArgV,SerialInput_Buffer,MAX_NUM_OF_INPUT_CHARACTERS,PredefTokenList);
//
//and cleared between each use (functions like TokenizeSerialInput() or TokenizeStringInput()):
//ClearTokenBuffer(ArgV);
//





//****************************
//Local copies of some standard c functions (so library isn't necessary)
//****************************
int Strlen(const char *str){
	char *s=(char*)str;
	while(*s)s++;
	return (int)(s-str);
}

//Needed for ParseForFunctionList()
char *Strcpy(char *dest, char *src){//has the advantage of being small and doesn't need a length input

	char *ret = dest;
	while (*dest++ = *src++) ;
	return ret;
}

//to do: make it so it starts copying just bytes until it gets to a boundary
void memcopy(void*MemoryDestination, void*MemorySource,int Bytes){
	int i,l;
    if(((unsigned int)MemoryDestination)<((unsigned int)MemorySource)){
        if(Bytes>=4){
            l=Bytes>>2;//length = bytes/4
            for(i=0;i<l;i++){
                ((int*)MemoryDestination)[i]=((int*)MemorySource)[i];
            }
            i=i<<2;//convert from 32bit to 8bit array coordinates by multiplying by 4
        }
        else{
            i=0;
        }
        for(;i<Bytes;i++){
            ((char*)MemoryDestination)[i]=((char*)MemorySource)[i];
        }
    }//end if(((unsigned int)MemoryDestination)<((unsigned int)MemorySource))
    else{//if going forward, copy from end so no overwriting in case that we are not moving too far
        l=Bytes-(Bytes&3);
        for(i=Bytes-1;i>=l;i--){
            ((char*)MemoryDestination)[i]=((char*)MemorySource)[i];
        }
        if(Bytes>=4){
            l=Bytes>>2;//length = bytes/4
            for(i=l-1;i>=0;i--){
                ((int*)MemoryDestination)[i]=((int*)MemorySource)[i];
            }
        }
    }//end if(((unsigned int)MemoryDestination)<((unsigned int)MemorySource)) else
}//end CopyMemory

//****************************
//end Local copies of some standard c functions
//****************************



//****************************
//Supporting functions for TokenizeInput(), TokenizeInit(),ClearTokenBuffer()
//****************************

char CharacterToInteger(char c){
	return ('0'<=(c) && (c)<='9')?(c)-'0':('a'<=(c) && (c)<='f')?(c)-'a'+10:('A'<=(c) && (c)<='F')?(c)-'A'+10:255;
}

char StringToNumber(const char *s, void *Number,char Base, char IsFloat){
	unsigned char digit,digits=0, IsNeg=0, IsDec=0;//, DecPntPos;
	int floatdiv=1;
	float f;
	unsigned char i=0;
	int number=0;
	while(s[i]==' ') i++;//get to first non-space character
	if(s[i]=='\n' || s[i+1]=='\n'){//only added this section because sscanf sets number to 0 if "\r\n" received
		*(int*)Number=0;
		i++;
		if(s[i+1]=='\n')
			i++;
	}
	else{
		digit=CharacterToInteger(s[i]);
		//while decimal/hex digit, or first character '-', or first '.'.
		while(digit!=255 || ( digits==0 && s[i]=='-') || (IsNeg==0 && s[i]=='.') ){
			if(s[i]=='-'){
				IsNeg=1;
			}
			else if(s[i]=='.'){
				IsDec=1;
				//DecPntPos=digits;
			}
			else{
				number=number*Base;
				number+=digit;
				digits++;
				if(IsDec)
					floatdiv=floatdiv*Base;
			}
			i++;
			digit=CharacterToInteger(s[i]);
		}
		if(digits){
			if(IsNeg)
				number=-number;
			if(IsFloat){
				f=number;
				f=f/floatdiv;
				*(float*)Number=f;
			}
			else{
				number=number/floatdiv;
				*(int*)Number=number;
			}
		}
 }
	if(digits){
		if('g'<=s[i] && s[i]<='z')
			digits=0;
	}
	return digits?i:0;//return place to look for next number
}






//#include <stdio.h>//if using printf instead of Printf

#include "SerialIO.h"
//
char TokenMatchLevel(char *Token, char *String, int Offset, int Length){
    char Match;
    int i=0;
    while(1){
        if(Token[i]==String[Offset+i]){
            if(i+1==Length){//
                if(Token[i+1]==0)
                    Match=2;//string segment ++ with token "++"
                else
                    Match=1;//string segment + with token "++"
                break;
            }
            else
                i++;
        }
        else{
            if(Token[i]==0)//ex: string segment +a with token "+" (might happen if previously trying to match with token ++
                Match=2;
            else
                Match=0;//string segment +a with token "++"
            break;
        }
    }
    return Match;
}

//
//An array of strings is constructed by listing pointers in beginning of allocated memory and placing strings
//in reverse order from end of buffer.
//

//
//Example of how information is stored:
//
//128 byte buffer, PredefTokenList[]={"++","+","-",""}.
//char **ArgV;
//char SerialInput_Buffer[128];//not necessary to initialize at 0, but assuming it for this example
//Right after TokenizeInit(&ArgV,SerialInput_Buffer,128,{"++","+","-",""}):
//0x408040: 03 00 00 00 bd 80 40 00|ba 80 40 00 b7 80 40 00    ......@...@...@.
//0x408050: 28 00 00 00 80 00 00 00|00 00 00 00 00 00 00 00    (...............
//0x408060: 00 00 00 00 00 00 00 00|b7 80 40 00 00 00 00 00    ..........@.....
//0x408070: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x408080: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x408090: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x4080a0: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x4080b0: 00 00 00 00 00 00 01 2d|00 01 2b 00 02 2b 2b 00    .......-..+..++.
//3 predefined tokens: "++" at space 0x004200bd, "+" at 0x004200ba, "=" at 0x004200b7.
//One character is reserved before each string for "type", though in the case of the
//predefined token list this is used to indicate the size of each string/token (2, 1, and 1).
//Though the pointers to the strings are filled in from the beginning, the actual strings
//these point to are filled in from the end of the memory buffer.  This way, the whole
//buffer can be used without having to guess at how big the average string will be.
//ArgV is 0x28 (40 in decimal) offset from beginning of buffer which is 0x80 (128)
//bytes long. Memory pointer ArgV[0] (which is where we will be putting the tokens that will
//be passed on to the parsing function) is initialized to the beginning of the last token ("-")
//entered from PredefinedTokenList[] (0x004080b7).
//
//User enters 1234+4321:
//0x408040: 03 00 00 00 bd 80 40 00|ba 80 40 00 b7 80 40 00    ......@...@...@.
//0x408050: 28 00 00 00 80 00 00 00|00 00 00 00 04 00 00 00    (...........@...
//0x408060: 04 00 00 00 02 00 00 00|b1 80 40 00 ae 80 40 00    ..........@...@.
//0x408070: ae 80 40 00 31 32 33 34|2b 34 00 00 34 33 32 31    ..@.1234+4..4321
//0x408080: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x408090: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x4080a0: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 2b 00    ..............+.
//0x4080b0: 00 31 32 33 34 00 01 2d|00 01 2b 00 02 2b 2b 00    .1234..-..+..++.
//When they reach "1234+4", the algorithm can be sure that it is looking at "+" and not just the
//beginning of a "++". The beginning "1234" is added to the ArgV[] token list at 0x004080b1,
//"+" is placed at 0x004080ae (before it in memory, though the pointer to it is 4 bytes to the right),
//and the place used for the temporary buffer is moved 8 bytes to the right. The last part ("4") is
//copied into the new location, followed (later) by "321" as user types them in.
//At this point, the character counter and the place where the algorithm is looking for
//new spacial tokens are both at 0x04, and the total number of tokens entered is 2 ("1234" and "+").
//The only defined token type in this version is 'q' for "string with quotes around it", so this
//field is zero for both.
//
//User presses ENTER ("\n" or "\r\n"):
//0x408040: 03 00 00 00 bd 80 40 00|ba 80 40 00 b7 80 40 00    ......@...@...@.
//0x408050: 28 00 00 00 80 00 00 00|00 00 00 00 00 00 00 00    (...........@...
//0x408060: 00 00 00 00 00 00 00 00|b1 80 40 00 ae 80 40 00    ..........@...@.
//0x408070: a8 80 40 00 a4 80 40 00|a4 80 40 00 34 33 32 31    ..@...@...@.4321
//0x408080: 0a 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x408090: 00 00 00 00 00 00 00 00|00 00 00 00 00 00 00 00    ................
//0x4080a0: 00 00 00 00 0d 0a 00 00|34 33 32 31 00 00 2b 00    ....\r\n..4321..+.
//0x4080b0: 00 31 32 33 34 00 01 2d|00 01 2b 00 02 2b 2b 00    .1234..-..+..++.
//'\n' or "\r\n" is separated from the rest of the token and are replaced with "\r\n" (unless
//there are no other tokens in which case "\n" is entered to indicate that the enter key IS the input).
//Both are entered (so we now have {"1234","+","4321","\r\n"}), everything is reset in perparation for
//the next line, and "4" is outputted to indicate to the parsing function that there is something
//for it to look at.
//
//Info1,Info2,...InfoLength,Parent,BufferSize,Length
//

#define ListLength(list) (*(((int*)(list))-1))
#define ListBufferSize(list) (*(((int*)(list))-2))
#define ListBuffer(list)((char*)&((list)[-4-ListInfoFieldsLength(list)]))
#define ListInfoFieldsLength(list) (*(((int*)(list))-4))
#define ListInfoFields(list) ((int*)&(list)[-4-ListInfoFieldsLength(list)])
#define ListParent(list)  (*(((char***)(list))-3)) //  ((char**)((list)[-3])) //ListInfoFields(list)-length of parent list
#define ListFreeSpace(list)  ( ((unsigned int)(list[ListLength(list)])) - ((unsigned int)(&(list[ListLength(list)+1]))) - 1 )//"-1" is for "type" indicator. includes extra string pointer

//AT some point implement ListInit and ListInitUsingBufferOf
void ListInit(char** *StringList, char *Buffer, int BufferSize, int NumOfInfoFields, char **ParentList){
    *StringList=(char**)(Buffer+sizeof(int)*(4+NumOfInfoFields));
    (*StringList)[0]=Buffer+BufferSize;
    ListLength(*StringList)=0;//number of elements stored
    ListBufferSize(*StringList)=BufferSize;
    ListParent(*StringList)=ParentList;//points to 0 or enclosing list
    ListInfoFieldsLength(*StringList)=NumOfInfoFields;
}
//Note: subtract 2 more (for type before first character of last string and the 0 at end of new string)
//if you want to see if a new string will fit.
#define ListBeginningOfFreeSpace(list) ((char*)&(list[ListLength(list)+1]))
void ListInitInsideBufferOf(char** *StringList, int NumOfInfoFields, char ** ParentList){
     char *Buffer=ListBeginningOfFreeSpace(ParentList);
     int BufferSize=ListFreeSpace(ParentList);
     ListInit(StringList, Buffer, BufferSize, NumOfInfoFields, ParentList);
}
char AddToken(char **StringList, char *text, int textSize, char type){//already used in regular code
    int ArgC;
    int rem=ListFreeSpace(StringList)-textSize;
    if(rem>=0){
        ArgC=ListLength(StringList);
        StringList[ArgC]-=(textSize+1);//subtract size of token plus 1 for terminating 0).
        StringList[ArgC][textSize]=0;//terminating 0 for token
        memcopy(StringList[ArgC],text,textSize);//fill token with characters
        StringList[ArgC][-1]=type;//text[-1]=token type
        StringList[ArgC+1]=StringList[ArgC]-1;//initialize next token pointer to end of remaining buffer (old token - 1 for its "type")(taken into account when looking at remaining space)
        ListLength(StringList)=(ListLength(StringList)+1);
        return 1;
    }
    else
        return 0;
}
//thinking of making function void AddInteger() to use buffer for list of integers in addition to list of strings
void ClearTokenBuffer(char **StringList){
    ListLength(StringList)=0;//list length = 0
    StringList[0]=ListBuffer(StringList)+ListBufferSize(StringList);//reset pointer
}

//Must be called before any other tokenizing functions:
void TokenizeInit(char** *ArgV,char *Buffer, int BufferSize, const char **PreDefinedTokens){
    int i;
    char *init;
    char *PredefinedTokenLengths=Buffer+1;
    int TokenNum;
    ListInit(ArgV, Buffer, BufferSize,0,0);
    for(TokenNum=0;PreDefinedTokens[TokenNum][0];TokenNum++){
        i=Strlen(PreDefinedTokens[TokenNum]);
        AddToken(*ArgV, (char*)PreDefinedTokens[TokenNum], i, i);
    }
    ListInitInsideBufferOf(ArgV, 3, *ArgV);
    for(i=0;i<ListInfoFieldsLength(*ArgV);i++){
        ListInfoFields(*ArgV)[i]=0;
    }
}

//****************************
//end Supporting functions for TokenizeInput(), TokenizeInit(),ClearTokenBuffer()
//****************************


//****************************
//Tokenizing functions
//****************************

int TokenizeInput(char **ArgV, int (*GetCharNonBlocking)(void*source),void *source){//returns ArgC when input is ready (when \n pressed)
    int NewChar=0;
    char InString=0,MatchLevel=0;
    char JustClosedString=0;
    int i=0,j,ArgC=0,Continue=1,TokenNum,TokenLoc=0;
    int NumOfPredefinedTokens;
    char *tempstr,*tempstrprev;//=TempStr+1;//before this, must put string lists at back of buffer instead of just after max number of inputs.
    int Overflow=0;
    char **PreDefinedTokens=ListParent(ArgV);
    NumOfPredefinedTokens=ListLength(PreDefinedTokens);

    ArgC=ListLength(ArgV);
    i=ListInfoFields(ArgV)[0];
    TokenLoc=ListInfoFields(ArgV)[1];
    InString=ListInfoFields(ArgV)[2];
    tempstr=((char*)&ArgV[ArgC+3])+1;//+3 (instead of +1) for case where predefined token analysis makes a+b -> a, +, and "b" in tempstr
                                 //since it won't be moved over till 2 new tokens are entered.
                                 //Extra "+1" so room for "type" at tempstr[-1]
    if(tempstr+i>ArgV[ArgC-1]-2)
        Overflow=1;
    while(Continue && !Overflow){
        NewChar=GetCharNonBlocking(source);
        if(NewChar>=0){
            if(ArgC==0 && i==0){
                //Printf("    %d tokens",NumOfPredefinedTokens);
                if(ArgC==0){
                    tempstr[-1]=0;//token type (for upcoming token)
                }
            }
            tempstr[i]=NewChar;//GetChar();
            //PutChar(tempstr[i]);
            //Check if in pre-defined token list:
            CheckAgain:
            if(!InString){
                MatchLevel=0;
                for(TokenNum=0;TokenNum<NumOfPredefinedTokens;TokenNum++){
                    MatchLevel=TokenMatchLevel((char*)PreDefinedTokens[TokenNum],tempstr,TokenLoc,i+1-TokenLoc);
                    //Printf("tok[%d] %s(%d)==",TokenNum,PreDefinedTokens[TokenNum],PredefinedTokenLengths[TokenNum]);
                    //PrintString(ArgV[ArgC]+TokenLoc,i+1-TokenLoc);
                    //Printf("(%d) match %d\r\n",i+1-TokenLoc,MatchLevel);
                    if(MatchLevel){
                        if(MatchLevel==2 && PreDefinedTokens[TokenNum][0]=='-' && PreDefinedTokens[TokenNum][1]==0 && TokenLoc==0 && CharacterToInteger(tempstr[TokenLoc+1])!=255)
                            MatchLevel=0;//if "-" token, there is already a space behind it, and the next character is numeric
                                        //ex: " -5..."
                        else
                            break;
                    }
                }
                if(MatchLevel==2){//a+b -> a, +, b //suppose ArgV[ArgC]=a+b...
                    if(TokenLoc>0){
                        AddToken(ArgV, tempstr, TokenLoc,tempstr[-1]);
                        tempstr[-1]=0;//reset type
                        ArgC++;
                    }
                    AddToken(ArgV, tempstr+TokenLoc, PreDefinedTokens[TokenNum][-1],0);
                    tempstr[-1]=0;//reset type
                    ArgC++;
                    tempstrprev=tempstr;
                    tempstr=((char*)&ArgV[ArgC+3])+1;//move tempstr over to new location
                    //if we went past end of token before recognizing it (due to first checking a longer special character)
                    if(i>TokenLoc+PreDefinedTokens[TokenNum][-1]-1){
                        memcopy(tempstr,tempstrprev+i,i-(TokenLoc+PreDefinedTokens[TokenNum][-1]-1));
                        i=(i-(TokenLoc+PreDefinedTokens[TokenNum][-1]-1))-1;
                        TokenLoc=0;
                        goto CheckAgain;//see if first character after predefined token is, itself, a predefined token
                    }
                    else{
                        i=0;
                        TokenLoc=0;
                    }
                }
                else if(MatchLevel==0){
                    TokenLoc++;
                }
            }//end if(!InString)
            //
            if(InString==1 && tempstr[i]=='\\'){
                InString=2;
            }
            else if(InString==2){
                if(tempstr[i]=='r')
                    tempstr[i]='\r';
                else if(tempstr[i]=='f')
                    tempstr[i]='\f';
                else if(tempstr[i]=='n')
                    tempstr[i]='\n';
                i++;
                InString=1;
            }
            else if(tempstr[i]=='\"' && InString<2) {
                if(InString){
                    InString=0;
                    TokenLoc=i;
                    JustClosedString=1;
                }
                else {
                    InString=1;
                    tempstr[-1]='\"';
                }
            }
            else if((tempstr[i]==' ') && !InString){
                if(i>0){//skips without i++ if begins with series of ' '
                    AddToken(ArgV, tempstr, i,tempstr[-1]);
                    tempstr[-1]=0;//reset type
                    ArgC++;i=0;TokenLoc=0;
                    tempstr=((char*)&ArgV[ArgC+3])+1;//move tempstr over to new location
                }
            } //i has 0, i+1 could have \", i+2 has next token
            else if(TokenLoc>0)//two places in "if(MatchLevel==2)" section that we don't want to increment here because i set to 0
                i++;//in quotes or not a ' '
            if((tempstr[i-1]=='\n') && !InString && !JustClosedString){//if last character entered was \n (and not part of quoted text)
                Continue=0;//note: the "JustClosedString" clause is in case "a b\n" is typed
                //\r\n or \f\n -> \n
                if(i>1){
                    if(tempstr[i-2]=='\r' || tempstr[i-2]=='\f'){
                        i--;
                    }
                }
                if(ArgC==0 && i==1) {//if this is the first character of the first token entered
                    AddToken(ArgV, tempstr, i,0);
                    tempstr[-1]=0;//reset type
                }
                else{//replace it with zero to terminate the present token, then enter \r\n as a new token
                    AddToken(ArgV, tempstr, i-1,tempstr[-1]);
                    ArgC++;
                    TokenLoc=0;
                    AddToken(ArgV,"\r\n", 2,0);//((char*)"\x00\r\n")+1
                }
                ArgC++;
                i=0;
            }//end if('\n')
            if(tempstr+i>ArgV[ArgC-1]-2)//see if extra character resulted in a buffer overflow condition
                Overflow=1;
            JustClosedString=0;
        }//end if(CharsAvail)
        else{
            break;
        }
    };//end while(Continue)
    ListInfoFields(ArgV)[0]=i;
    ListInfoFields(ArgV)[1]=TokenLoc;
    ListInfoFields(ArgV)[2]=InString;
    if(Overflow)
        return -1;
    if(Continue==0){//only happens when reached end of line.
        return ArgC;// (\n encountered).
    }
    else{
        return 0;//just ran out of characters (without \n character), so might be more input later
    }
}//end TokenizeInput

int TokenizeSerialInput_GetSerialCharAndEcho(void*source){
    int c = GetCharNonBlocking();
    if(c>=0)
        PutChar(c);
    return c;
}
int TokenizeSerialInput(char **ArgV){
    return TokenizeInput(ArgV,&TokenizeSerialInput_GetSerialCharAndEcho,0);
}
//
struct TokenizeStringInput_CharSource{
        int pos=0;
        char *str;
    };
int TokenizeStringInput_GetChar(void*source){
    int c;
    char *s=((struct TokenizeStringInput_CharSource*)source)->str;
    int n=((struct TokenizeStringInput_CharSource*)source)->pos;
    if(s[n]){
        c=s[n];
        ((struct TokenizeStringInput_CharSource*)source)->pos++;
    }
    else
        c=-1;
    return c;
}
int TokenizeStringInput(char **ArgV, char *str){
    struct TokenizeStringInput_CharSource chars;
    chars.pos=0;
    chars.str=str;
    return TokenizeInput(ArgV,&TokenizeStringInput_GetChar,&chars);
}

//****************************
//end Tokenizing functions
//****************************












//****************************
//Parsing functions
//****************************



//function string:
//"%s repeat onhigh/onlow %d %d"
//inputs:
//{"Flash_En0","repeat","OnLow","3","10"}
//outputs:
//{&StrVarStorage[n],0,1,3,10}

//The range for single-precision floating-point values is:
//(float): The range for single-precision floating-point values is
//1.17549435e–38 to 3.40282347e+38
//(double): The range for double-precision floating-point values is:
//2.22507385850720138e–308 to 1.79769313486231571e+308



//note: 0 includes isolated strings and "\r\n" (end of line) or "\n" (end of line when no other tokens entered before ENTER pressed)
#define Int_Var 1
#define Str_Var 2	  //Var=variable
#define Str_Enum 3	  //Enum=enumerated(in this case, it just means that more than one string literal is accepted as input)
#define Str_Lit 4	  //Lit=literal	  [only used in GetTokenValue()]	  (in "add %d %d",    "add" is a Str_Lit)
#define No_Tokens_Left 6 //				 [only used in GetTokenValue()]
#define Start_Multi 7
#define Int_Const 8  //something like %4 which tells function which function input string it was called from


int GetTokenValue(char *ArgVtoken, const char *FunctionSyntaxString,int *StringLocation, char *TokenType){
	int EndPos,i=0,IntValue=0,EnumValue=0,EnumNum,TokenLength;
	char NotLastEnum;
	char IsMultiValue=0;
    int StrLoc=*StringLocation;
	//in case we were dropped in whitespace
	while((FunctionSyntaxString[StrLoc]==' ' || FunctionSyntaxString[StrLoc]=='/') && FunctionSyntaxString[StrLoc]!=0){
		(StrLoc)++;
	}
	EndPos=StrLoc;
	if(FunctionSyntaxString[StrLoc]=='%'){
		while(FunctionSyntaxString[EndPos]!=' ' && FunctionSyntaxString[EndPos]!=0)
            EndPos++;//find end of function input token at StrLoc
		if(EndPos-StrLoc==2){//"%_ "
			if(FunctionSyntaxString[StrLoc+1]=='d'){	  //%d case
				if(ArgVtoken[0]!='\r' && ArgVtoken[0]!='\n'){//sscanf() seems to accept "\r\n" as value 0
					if(ArgVtoken[1]=='x'){
						if(StringToNumber((char*)ArgVtoken+2, &IntValue,16, 0))
                            *TokenType=Int_Var;
						else *TokenType=0;
					}
					else if(ArgVtoken[1]=='b'){
						if(StringToNumber((char*)ArgVtoken+2, &IntValue,2, 0))
                            *TokenType=Int_Var;
						else
                            *TokenType=0;
					}
					else if(StringToNumber((char*)ArgVtoken, &IntValue,10, 0))
                        *TokenType=Int_Var;			  //decimal
					else
                        *TokenType=0;
				}//end if(ArgVtoken[0]!='\r' && ArgVtoken[0]!='\n')
				else
					*TokenType=0;
			}//end if(FunctionSyntaxString[StrLoc+1]=='d')
			else if(FunctionSyntaxString[StrLoc+1]=='s')
				*TokenType=Str_Var;
			else if(FunctionSyntaxString[StrLoc+1]=='q'){	//quoted string
//				printf("\r\n____found %q____%s\r\n",ArgVtoken-1);
				if(ArgVtoken[-1]=='\"')
					*TokenType=Str_Var;
				else
					*TokenType=0;
			}
			else if(FunctionSyntaxString[StrLoc+1]=='b'){	 //binary number
				if(StringToNumber((char*)ArgVtoken, &IntValue,2, 0))//if(BinaryStringToNumber(ArgVtoken,&IntValue))
                    *TokenType=Int_Var;
				else
                    *TokenType=0;
			}
			else if(FunctionSyntaxString[StrLoc+1]=='x'){	 //hex number
				if(StringToNumber((char*)ArgVtoken, &IntValue,16, 0))//if(Sscanf(ArgVtoken,"%x",&IntValue))
                    *TokenType=Int_Var;
				else
                    *TokenType=0;
			}
			else if(FunctionSyntaxString[StrLoc+1]=='f'){	 //floating point decimal number
                if(ArgVtoken[1]=='x'){
                    if(StringToNumber((char*)ArgVtoken+2, &IntValue,16, 1))
                        *TokenType=Int_Var;
                    else *TokenType=0;
                }
                else if(ArgVtoken[1]=='b'){
                    if(StringToNumber((char*)ArgVtoken+2, &IntValue,2, 1))
                        *TokenType=Int_Var;
                    else
                        *TokenType=0;
                }
				else if(StringToNumber((char*)ArgVtoken, &IntValue,10, 1))//if(Sscanf(ArgVtoken,"%f",(float*)&IntValue))
                    *TokenType=Int_Var;
                else
                    *TokenType=0;
			}
			else if(FunctionSyntaxString[StrLoc+1]=='m'){	 //multi input section
				*TokenType=Start_Multi;
			}
			else if('0'<=FunctionSyntaxString[StrLoc+1] && FunctionSyntaxString[StrLoc+1]<='9'){//#define Int_Const 8
				if(StringToNumber(FunctionSyntaxString+StrLoc+1, &IntValue,10, 0))//if(Sscanf(FunctionSyntaxString+StrLoc+1,"%d",&IntValue))
					*TokenType=Int_Const;
				else
                    *TokenType=0;
			}
			else
				*TokenType=0;
		}//end if(EndPos-StrLoc==2)
		else
			*TokenType=0;
	}//end if(FunctionSyntaxString[StrLoc]=='%')
	else if(FunctionSyntaxString[StrLoc]!=0){
		//test match and determine token end pos
		EnumValue=-1;
		EnumNum=0;
		do{
			//test match
			i=0;
			while(ArgVtoken[i]!=0 && FunctionSyntaxString[EndPos]==ArgVtoken[i]){EndPos++;i++;};
			if(ArgVtoken[i]==0 && EndPos>StrLoc) //Match!!!
				EnumValue=EnumNum;
			else				//go to end of element of enum list string [ex: if in "apple", go to "/" apple/orange/grape]
				while(FunctionSyntaxString[EndPos]!=' ' && FunctionSyntaxString[EndPos]!=0 && FunctionSyntaxString[EndPos]!='/') EndPos++;
			if(FunctionSyntaxString[EndPos]=='/'){ //so there are more possible matches
				IsMultiValue=1;
				NotLastEnum=1;
				EndPos++;//to get to first character of next string		 [from "/" to "o"]
				EnumNum++;
			}
			else
				NotLastEnum=0;

		}while(NotLastEnum && EnumValue<0);
		//if first one was match, see if this is the only element in the enum list.
		if(EnumValue==0 && !IsMultiValue){
			while(FunctionSyntaxString[EndPos]!=' ' && FunctionSyntaxString[EndPos]!=0 && FunctionSyntaxString[EndPos]!='/') EndPos++;
			if(FunctionSyntaxString[EndPos]=='/') IsMultiValue=1;
		}
		//go to end of enum list
		while(FunctionSyntaxString[EndPos]!=' ' && FunctionSyntaxString[EndPos]!=0) EndPos++;
		//set to enum type or null type
		if(EnumValue>=0){if(IsMultiValue)*TokenType=Str_Enum; else *TokenType=Str_Lit;}
		else *TokenType=0;
	}//end if(FunctionSyntaxString[StrLoc]!=0)
	else{
		*TokenType=No_Tokens_Left;
	}

	if(FunctionSyntaxString[EndPos]==0) *StringLocation=EndPos;
	else *StringLocation=EndPos+1;

	if(*TokenType==Str_Enum) return EnumValue;		//return input number banana=2 if orange/apple/banana/grape input
	else if(*TokenType==Int_Var || *TokenType==Int_Const) return IntValue; //return integer scanned for %d input
	else if (*TokenType==Str_Var){				  //return user entered string for %s input
		return (int)ArgVtoken;
	}//end if (*TokenType==Str_Var)
	else if (*TokenType==Start_Multi){
		return 0;
	}
	else if(*TokenType==No_Tokens_Left) return 0;
	else{ return -1;}//==0
}//end GetTokenValue()



//just checks one function in list:
int ParseForFunction(char **Argv_,int argc_, int ArgVNumInit, const char *SyntaxString, int* args_, int *argnum_){//make last element have InputDescription=="" and Function==0
	//note: FunctStringLocation=location within function input string
	char token_type;
	int token_value;
	int TokenNum;
	int StrLocTemp;
	int i=0;
	int MultiVariableStringLocation=-1,MultiVariableLengthLocation=-1,MultiEndStringLocation;//note: can't know what initial value of MultiEndStringLocation will be
	int FunctStringLocation=0;
	*argnum_=0;
	TokenNum=ArgVNumInit;
    while(TokenNum<argc_){//not end of list of functions  [and no functions over 10 arguments]
        if(MultiVariableLengthLocation==-1)//if not an "%m" section	   [put breakpoint here for troubleshooting]
            token_value=GetTokenValue(Argv_[TokenNum], SyntaxString,&FunctStringLocation,&token_type);
        else{ //look for end token, then look for multi/list element/variable token
            FunctStringLocation=MultiEndStringLocation;//in %m example below, this would be the location of "end"
            if(SyntaxString[MultiEndStringLocation]!=0)//if %m section has a non-empty end-token
                token_value=GetTokenValue(Argv_[TokenNum], SyntaxString,&FunctStringLocation,&token_type);
            else
                token_type=0;
            if(token_type==0){//if not the "end" token (see %m example below)
                FunctStringLocation=MultiVariableStringLocation;//in %m example below, this would be the location of "%d"
                token_value=GetTokenValue(Argv_[TokenNum], SyntaxString,&FunctStringLocation,&token_type);
                if(token_type!=0)//if token is of an expected type (an integer(%d) in example below)
                    args_[MultiVariableLengthLocation]=args_[MultiVariableLengthLocation]+1;
                else{
                    if(SyntaxString[MultiEndStringLocation]!=0)//if %m section has a non-empty end-token
                        MultiVariableLengthLocation=-1;
                    else
                        break;//return TokenNum;
                }
            }
            else
                MultiVariableLengthLocation=-1;
        }
        if(token_type==No_Tokens_Left){	  //end of function string. function entered
            break;//return TokenNum;
        }
        else if(token_type==0){	 //function didn't work, start over with next function at TokenNum=TokenNumBase
            TokenNum=-1;break;//return -1;
        }
        else{
            //%m (multi-input) example:
            // function string = "add %m %d end"
            // token list = {"add","5","9","7","end"}
            // function arguments = {3,5,9,7}
            //Note: can also do "add %m %d \r\n" or even "add %m %d")
            //(if a newline is the only token, token list = {"\n"})
            if(token_type==Start_Multi){//if this token is "%m"
                MultiVariableStringLocation=FunctStringLocation; //store location of variable string (in example: "%d")
                GetTokenValue((char*)"", SyntaxString,&FunctStringLocation,&token_type);//use to find next token in function input string. "char*" type cast in case it is compiled in a unicode environment
                MultiEndStringLocation=FunctStringLocation;//store location of end string.(in example: "end")
                FunctStringLocation=MultiVariableStringLocation;token_type=Start_Multi;//reset values to original
                args_[*argnum_]=0;//will be incremented as new elements added to the list-input variable section
                MultiVariableLengthLocation=*argnum_;
                (*argnum_)++;
                TokenNum--;//to cancel out the TokenNum++ below. %m does not absorb a token.
            }
            //token entered into function argument list
            else if(token_type!=Str_Lit){//  examples of what applies: a/b/c, %d, %b, %f, %3
                args_[*argnum_]=token_value; //put breakpoint in here if troubleshooting
                (*argnum_)++;
                if(token_type==Int_Const)
                    TokenNum--;//to cancel out the TokenNum++ below. function string elements like "%3" don't get their value from a token
            }
            //increment TokenNum
            TokenNum++;
            if(TokenNum==argc_){//in case current token is the last one entered
                StrLocTemp=FunctStringLocation;
                token_value=GetTokenValue((char*)" ", SyntaxString,&FunctStringLocation,&token_type);//(char*)" " is in case it is compiled in a unicode environment
                if(token_type!=No_Tokens_Left)
                    TokenNum=-1;
                break;
            }//end if(TokenNum==argc_)
        }//end if(token_type==0) else
    }//end while(TokenNum<argc_)
    return TokenNum;
}//end ParseForFunction()


//function list example:
//FunctionDescriptionType MathFunctionList[]={
//	{"exit",&ExitFunction},
//	{"sum %m %d \r\n", &AddAll,0},   //"sum 1 2 3 4" -> {4,1,2,3,4} => 1+2+3+4=10
//	{"inverse %1 %f",&PrintOneOverFloat,"inverse [float], opt: shorten. Ex: 'inv 10 sh' -> 0.100"},   //"inverse 10" -> {1,1092616192} (depending on how floats are stored in your system)
//	{"inverse %0 %f shorten",&PrintOneOverFloat,""},   //"inverse 10 shorten" -> {0,1092616192} -> 1/10. -> .1
//	{"%d +/- %d", &AddOrSubtract,0},   //"3-4" -> {3,1,4}, "3+4" -> {3,0,4} -> 3+4=7
//	{"length %s",&StringLengthFunction},   //The number passed should be interpreted (by StringLengthFunction) as a pointer to a string.
//	{"",0}
//};

//Debugging suggestions: Argv_[TokenNum] , FunctNum_ , FunctList[*FunctNum_].Input	   FunctList[*FunctNum_].Input+*FunctStringLocation
void ParseForFunctionList(char **Argv_,int argc_, FunctionDescriptionType *FunctList, char *MenuName){//make last element have InputDescription=="" and Function==0
	//note: FunctStringLocation=location within function input string
	char token_type;
	int token_value;
	int TokenNumBase,TokenNum;
	int StrLocTemp;
	int i=0,n;
	int MultiVariableStringLocation=-1,MultiVariableLengthLocation=-1,MultiEndStringLocation;//note: can't know what initial value of MultiEndStringLocation will be
	int FunctNum_=0;
	int argnum_=0;
	int args_[MAX_NUM_OF_FUNCTION_ARGUMENTS];

	TokenNumBase=0;
	TokenNum=TokenNumBase;
	if(argc_>0 && Argv_[0][0]=='?' && Argv_[0][1]=='?' && Argv_[0][2]==0){
		printf("\r\nFunction input format list: \r\n");
		while(FunctList[i].Function){
			if(FunctList[i].Input[0]=='\n') printf("\\n\r\n");
			else printf("%s\r\n",FunctList[i].Input);
			i++;
		}
		printf("\r\n");
	}
	else if(argc_>0 && Argv_[0][0]=='?' && Argv_[0][1]==0){
		printf("\r\n");
		i=0;
		while(FunctList[i].Function){
			if(FunctList[i].Description){
				if(FunctList[i].Description[0])//if not just ""
					printf("%s\r\n",FunctList[i].Description);
			}
			else
				printf("%s\r\n",FunctList[i].Input);
			i++;
		}
		if(MenuName)
			printf("?: Print %s menu options\r\n",MenuName);
	}
	else{
//		printf(" ArgV[]= ");for(i=0;i<argc_;i++)printf("%s ",Argv_[i]);printf("\r\n");
//		for(i=Argv_[0]-((char*)Argv_);i<Argv_[argc_]-((char*)Argv_);i++)(((char*)Argv_)[i]>' '&&((char*)Argv_)[i]<='~'?printf("%c",((unsigned char*)Argv_)[i]):printf(" \\%d ",((unsigned char*)Argv_)[i]));;printf("\r\n");
		while(FunctList[FunctNum_].Function && argnum_<=MAX_NUM_OF_FUNCTION_ARGUMENTS && TokenNum<argc_){	//not end of list of functions  [and no functions over 10 arguments]
//			printf("GetTokenValue(argv[%d]=\"%s\",\"%s\",%d) \r\n",TokenNum,Argv_[TokenNum],FunctList[*FunctNum_].Input,*FunctStringLocation);
            n=ParseForFunction(Argv_, argc_, TokenNum,
                            FunctList[FunctNum_].Input, args_,
                            &argnum_);
            if(n>=0){
                TokenNum=n;
                FunctList[FunctNum_].Function(args_);
                FunctNum_=0;
            }
            else{
                FunctNum_++;
            }
		}
	}
}//end ParseForFunctionList()


//****************************
//end Parsing functions
//****************************




