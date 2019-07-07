#include "proc.h"

char *GetProcessCmdLine(char *RetStr, pid_t pid)
{
STREAM *S;
struct stat Stat;
char *Tempstr=NULL;
int i, len=2048;

Tempstr=FormatStr(Tempstr, "/proc/%d/cmdline", pid);
S=STREAMOpen(Tempstr, "r");
if (S)
{
  Tempstr=SetStrLen(Tempstr, len);
  len=STREAMReadBytes(S, Tempstr, len);
	Tempstr[len]='\0';
  for (i=0; i < len; i++) if (Tempstr[i]=='\0') Tempstr[i]=' ';
  STREAMClose(S);
}

RetStr=CopyStr(RetStr, Tempstr);
StripTrailingWhitespace(RetStr);
Destroy(Tempstr);

return(RetStr);
}
