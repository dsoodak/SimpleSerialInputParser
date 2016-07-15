
Simple serial input menu designed to not take up too much memory, and to be able to quickly & easily
create & enter serial debugging commands of medium complexity in a format reminiscent of the printf() function.

After re-creating this a couple of times for firmware debugging, I decided to open source a version of it
to save myself the trouble.

"%d +/- %d" will look for 2 integers with either + or - in-between which will be represented by 0 or 1),
there is an option to automatically put separate certain tokens so you don't have to over-use the space-bar
when typing. I also added the rule that a space followed by - and a number automatically interpret
it as a minus sign so "3+ -4" will be tokenized as {3,+,-4} instead of {3,+,-,4}. You only have to
type part of a token and it will look for the first match (ex: a syntax string "inverse $f" will be matched
if you type in "in .125" as long as you don't have a syntax string like "increment %f" listed before it).

On some chips it can be tricky and or space consuming to set up dynamic memory allocation or to include a lot of
standard c/c++ libraries, so this project only assumes the existence of a c compiler and something equivalent to
a serial port.

It uses Printf() which has the advantages that it can do binary output, and you don't need to include a whole
library to use it. It is also a good starting point if you want to make your own custom printf() function.
SerialIO.c,.h is written for use in windows DOS mode (in other environments, just define your own versions of
PutChar() and GetCharNonBlocking() (which returns -1 if serial buffer empty)). When calling

Dustin Soodak
dsoodak yahoo mail (p
