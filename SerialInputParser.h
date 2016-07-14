
//Dustin Soodak
//dsoodak yahoo

#ifndef SERIAL_INPUT_PARSER_h
#define SERIAL_INPUT_PARSER_h

//
//Simple serial input menu designed to not take up too much memory, and to be able to quickly & easily
//create & enter serial debugging commands of medium complexity in a format reminiscent of the printf() function.
//
//After re-creating this a couple of times for firmware debugging, I decided to open source a version of it
//to save myself the trouble.
//
//"%d +/- %d" will look for 2 integers with either + or - in-between which will be represented by 0 or 1),
//there is an option to automatically put separate certain tokens so you don't have to over-use the space-bar
//when typing. I also added the rule that a space followed by - and a number automatically interpret
//it as a minus sign so "3+ -4" will be tokenized as {3,+,-4} instead of {3,+,-,4}. You only have to
//type part of a token and it will look for the first match (ex: a syntax string "inverse $f" will be matched
//if you type in "in .125" as long as you don't have a syntax string like "increment %f" listed before it).
//
//On some chips it can be tricky and or space consuming to set up dynamic memory allocation or to include a lot of
//standard c/c++ libraries, so this project only assumes the existence of a c compiler and something equivalent to
//a serial port.
//
//It uses Printf() which has the advantages that it can do binary output, and you don't need to include a whole
//library to use it. It is also a good starting point if you want to make your own custom printf() function.
//SerialIO.c,.h is written for use in windows DOS mode (in other environments, just define your own versions of
//PutChar() and GetCharNonBlocking() (which returns -1 if serial buffer empty)). When calling
//
//Dustin Soodak
//dsoodak yahoo mail (put "github" in subject)
//

#define MAX_NUM_OF_FUNCTION_ARGUMENTS 10 //used inside ParseForFunctionList() function.



extern void TokenizeInit(char** *ArgV,char *Buffer, int BufferSize, const char **PreDefinedTokens);
extern void ClearTokenBuffer(char **StringList);
extern int TokenizeInput(char **ArgV,int (*GetCharNonBlocking)(void*source),void *source);

//These use TokenizeInput:
extern int TokenizeSerialInput(char **ArgV);
extern int TokenizeStringInput(char **ArgV, char *str);


extern int ParseForFunction(char **Argv_,int argc_, int ArgVNumInit, const char *SyntaxString, int* args_, int *argnum_);
/*
//Example with TokenizeStringInput() and ParseForFunction()
//
#include "SerialInputParser.h"
#include "Printf.h" //or #include <cstdio>
#define printf Printf
//
#define MAX_NUM_OF_INPUT_CHARACTERS 256
char SerialInput_Buffer[MAX_NUM_OF_INPUT_CHARACTERS];
//
int main(){
    int i;
    char **ArgV;
	int ArgC=0;
	int TestTokensUsed=0;
    const char *PredefTokenList[]={"++","+","-",""};
    int TestOutputArgs[10];
    int TestArgNum=0;
    //not necessary for algorithm but useful for debugging the buffer functions
    for(i=0;i<MAX_NUM_OF_INPUT_CHARACTERS;i++)
        SerialInput_Buffer[i]=0;
    //Must be called before any other tokenizing functions:
    TokenizeInit(&ArgV,SerialInput_Buffer,MAX_NUM_OF_INPUT_CHARACTERS,PredefTokenList);
    //Tokenize an input string and see if a particular function string matches:
    ArgC=TokenizeStringInput(ArgV, "1234+4 inv .2\n");
    TestTokensUsed=ParseForFunction(ArgV, ArgC, 0, "%d +/- %d", TestOutputArgs,&TestArgNum);
    printf("After tokenizing \\\"%s\\\", ParseForFunction (with function string \\\"\\%d +/- \\%d\\\") \r\nused tokens ","1234+4 inv .2\\n");
    for(i=0;i<TestTokensUsed;i++){
        printf("%s%s",ArgV[i],i<TestTokensUsed-1?",":"");
    }
    printf(" and converted them to integers");
    for(i=0;i<TestArgNum;i++){
        printf("%d%s",TestOutputArgs[i],i<TestArgNum-1?",":"");
    }
}
*/



typedef struct
{
    const char* Input;
    void  (* Function)(int *ArgumentList);//void  (* Function)(Int *ArgumentList, int ArgumentListLength);//"Reduce Program Size". change to constant pointer to function(actually, can't figure out how to do it here)//void  (*Function)(int *ArgumentList, int ArgumentListLength);
	const char* Description;//Printed when "?" token received. Put 0 to make it just display the input string, or "" to print out nothing.
} FunctionDescriptionType;

extern void ParseForFunctionList(char **Argv_,int argc_, FunctionDescriptionType *FunctList, char *MenuName);
/*
//Example with TokenizeSerialInput() and ParseForFunctionList()
//
#include "SerialInputParser.h"
#include "Printf.h" //or #include <cstdio>
#define printf Printf
//
char Continue=1;
void ExitFunction(int *ArgumentList){
	Continue=0;
	printf("ExitFunction is exiting\r\n");
}
void AddAll(int *ArgumentList){
    int i,sum=0;
    for(i=0;i<ArgumentList[0];i++)
        sum+=ArgumentList[1+i];
	printf("Sum(%d): %d\r\n",ArgumentList[0],sum);
}
void PrintOneOverFloat(int *ArgumentList){
    float value;
    value=*(float*)&(ArgumentList[1]);
    value=1/value;
    if(ArgumentList[0]==0)
        printf("%0.3f\r\n",value);
    else
        printf("%0.8f\r\n",value);

}
void AddOrSubtract(int *ArgumentList){
	if(ArgumentList[1]==0)  printf("Sum: %d\r\n",ArgumentList[0]+ArgumentList[2]);
	if(ArgumentList[1]==1)  printf("Difference: %d\r\n",ArgumentList[0]-ArgumentList[2]);
}
void StringLengthFunction(int *ArgumentList){
    int i=0;while(((char *)(ArgumentList[0]))[i])i++;
	printf("String Length(%s): %d\r\n",(char *)(ArgumentList[0]),i);
}
FunctionDescriptionType MathFunctionList[]={
	{"exit",&ExitFunction,0},
	{"sum %m %d", &AddAll,0},
	{"inverse %0 %f shorten",&PrintOneOverFloat,"inverse [float], opt: shorten. Ex: 'inv 10 sh' -> 0.100"},
	{"inverse %1 %f",&PrintOneOverFloat,""},
	{"%d +/- %d", &AddOrSubtract,0},
	{"length %s",&StringLengthFunction,0},
	{"",0}
};
//
#define MAX_NUM_OF_INPUT_CHARACTERS 256
char SerialInput_Buffer[MAX_NUM_OF_INPUT_CHARACTERS];
char **ArgV;
int ArgC=0;
const char *PredefTokenList[]={"++","+","-",""};
//
int main(){
    //Must be called before any other tokenizing functions:
    TokenizeInit(&ArgV,SerialInput_Buffer,MAX_NUM_OF_INPUT_CHARACTERS,PredefTokenList);
	while(Continue){
        ArgC=TokenizeSerialInput(ArgV);
        if(ArgC>0){
            ParseForFunctionList(ArgV,ArgC, MathFunctionList, "math menu");
            ClearTokenBuffer(ArgV);
        }//end if(ArgC)
	}
}
*/







#endif	  //#ifndef SERIAL_INPUT_MENU_h


