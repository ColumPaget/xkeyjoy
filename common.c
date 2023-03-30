#include "common.h"


int BitIsSet(void *BitMask, int BitPos, int size)
{
    unsigned int bit;
    int segment;

    //segment is the byte within our bitmask
    segment=BitPos / 8;

    //if we are asking for a byte bitmask, then return false
    if (size <= segment) return(FALSE);

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

