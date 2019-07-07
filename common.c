#include "common.h"


int BitIsSet(void *BitMask, int BitPos)
{
unsigned int bit;
int segment;

segment=BitPos / 8;
bit=1 << (BitPos %8 );
return ( ((uint8_t *)BitMask)[segment] & bit);
}


char *ReadFile(char *RetStr, const char *Path)
{
STREAM *S;

S=STREAMOpen(Path, "r");
if (S)
{
  RetStr=STREAMReadLine(RetStr, S);
  STREAMClose(S);
}
StripTrailingWhitespace(RetStr);

return(RetStr);
}

