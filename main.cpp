

#include "SerialInputParser.h"

#include "Printf.h" //or #include <cstdio>
#define printf Printf

//Dustin Soodak
//dsoodak yahoo mail (put "github" in subject)


//for debug
void PrintStrings(char **strings,int length){
    int i;
    for(i=0;i<length;i++){
        if(i==0) printf("{");
        if(i<length-1) printf("%s,",strings[i]);
        else printf("%s}",strings[i]);
    }
}


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

#define MAX_NUM_OF_INPUT_CHARACTERS 256


char SerialInput_Buffer[MAX_NUM_OF_INPUT_CHARACTERS];


int main(){

    int i;

	char **ArgV;
	int ArgC=0;
	int TestTokensUsed=0;
    const char *PredefTokenList[]={"++","+","-",""};

    int TestOutputArgs[10];
    int TestArgNum=0;


    //Testing new functionality of printf:
    Printf("Printing the float 15.125 in binary & hex:  %0.3b, %0.3X\r\n\r\n",15.125,15.125);


    //not necessary for algorithm but useful for debugging
    for(i=0;i<MAX_NUM_OF_INPUT_CHARACTERS;i++)
        SerialInput_Buffer[i]=0;

    //Must be called before any other tokenizing functions:
    TokenizeInit(&ArgV,SerialInput_Buffer,MAX_NUM_OF_INPUT_CHARACTERS,PredefTokenList);

    //Tokenize a string and automatically search for a function that can accept it as input:
    printf("Test tokenizing a string '3+4\\\\n' and calling appropriate function:\r\n");
    printf("\"3+4\n\" -> TokenizeStringInput -> ");
    ArgC=TokenizeStringInput(ArgV, "3+4\n");
    PrintStrings(ArgV,ArgC);
    printf("\r\n");
    printf("-> ParseForFunctionList which calls AddOrSubtract (with function syntax string is '\\%d +/- \\%d') which prints\r\n");
    ParseForFunctionList(ArgV,ArgC, MathFunctionList, "math menu");
    printf("\r\n");
    ClearTokenBuffer(ArgV);

    //Tokenize a string and see if a particular function string matches:
    ArgC=TokenizeStringInput(ArgV, "inv .2 3+4\n");
    TestTokensUsed=ParseForFunction(ArgV, ArgC, 0, "inverse %1 %f", TestOutputArgs,&TestArgNum);
    printf("After tokenizing \\\"%s\\\", ParseForFunction (with function syntax string \\\"inverse \\%1 \\%f\\\") used \r\ntokens ","inv .2 3+4\\n");
    PrintStrings(ArgV,TestTokensUsed);
    printf(" and converted them to ");
    for(i=0;i<TestArgNum;i++){
        printf("%d%s",TestOutputArgs[i],i<TestArgNum-1?",":"");
    }
    printf(" (second argument as float: %0.3f)\r\n\r\n",*(float*)&TestOutputArgs[1]);
    ClearTokenBuffer(ArgV);
    //
    //
	while(Continue){
        ArgC=TokenizeSerialInput(ArgV);
        if(ArgC>0){
            PrintStrings(ArgV,ArgC);
            printf("\r\n");
            ParseForFunctionList(ArgV,ArgC, MathFunctionList, "math menu");
            ClearTokenBuffer(ArgV);

        }//end if(ArgC)
	}
}

