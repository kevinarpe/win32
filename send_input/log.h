#ifndef H_LOG
#define H_LOG

#include <stdio.h>  // required for FILE

void Log(FILE *fp, const char *lpszMsg);
void LogF(FILE *fp, const char *lpszMsgFmt, ...);
void LogFV(FILE *fp, const char *lpszMsgFmt, va_list ap);

#endif  // H_LOG

