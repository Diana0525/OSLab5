#include "disk.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void runcmd(char *argv[],int argc);
int sh();
int getcmd(char *buf, int nbuf);
void clearcmd(char* cmd, char* argv[],int* argc);
int clearDir(char *dir, char *clear_dir[]);

