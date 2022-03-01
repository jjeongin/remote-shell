// Remote shell project [Phase 1] 
// Date: Feb 26, 2022
// Author: Jeongin Lee

// Command list : ls, pwd, mkdir, rm, cd, clear, cat, find, echo, mv


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdio.h>
#include "command.h"



void ls(char ** args) {
	execvp("ls", args);
}