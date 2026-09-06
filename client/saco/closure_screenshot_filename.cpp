// Source0.2.5 subclass.cpp287 adapted to observed R5 screenshot naming utility.
// Static matching only: never call this candidate or create screenshot files.
#include "main.h"
#include <string>
#include <stdio.h>
extern CHAR szSAMPDir[MAX_PATH+1];
PCHAR GetSAMPPath() {
 return szSAMPDir;
}
int GetScreenshotFileName(std::string &fileName) {
 fileName=GetSAMPPath();
 char buffer[MAX_PATH]={0};
 WIN32_FIND_DATA findData;
 HANDLE handle;
 int index;
 for(index=0;index<1000;index++) {
  sprintf(buffer,(fileName+"\\screens\\sa-mp-%03i.png").c_str(),index);
  handle=FindFirstFile(buffer,&findData);
  if(handle!=INVALID_HANDLE_VALUE)FindClose(handle);
  else break;
 }
 fileName=buffer;
 return index;
}
