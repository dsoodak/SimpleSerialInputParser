#ifndef PRINTF
#define PRINTF


//Dustin Soodak
//dsoodak yahoo mail (put "github" in subject)


//Instead of #include <cstdio>,  use #include "Printf.h" and #define Printf printf.
//This is useful if you
//1. Are working with a chip that has limited space and don't want to include all of
//   stdio just to print numbers to a serial port.
//2. Want to see things like chip registers printed in binary, and have the option
//   to print floats in binary or hex.
//3. Wish to add your own custom functionality to printf.

extern void Printf(const char *pcFullString, ...);

//Ex:
/*
#include "Printf.h" //instead of #include <cstdio>
#define Printf printf
int main(){
    printf("%0.3f\r\n",15.125);
    printf("%0.3x\r\n",15.125);
    printf("%0.3b\r\n",15.125);
}
*/


#endif // PRINTF

