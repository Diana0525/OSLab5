#include "disk.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void runcmd(char *argv[],int argc);
int sh();
int getcmd(char *buf, int nbuf);
int fork1(void);
void panic(char *s);
void clearcmd(char* cmd, char* argv[],int* argc);
int clearDir(char *dir, char *clear_dir[]);

