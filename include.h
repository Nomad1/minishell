/**
  Copyright © 2019 Odzhan. All Rights Reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. The name of the author may not be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY AUTHORS "AS IS" AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <glob.h>

#include <link.h>
#include <elf.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <poll.h>

// write helpers

#define PRINT_TEXT(s, text) write(s, text, sizeof(text));
#define PRINT_STR(s, text) write(s, text, strlen(text));
#define PRINT_LEN(s, text, len) write(s, text, len);
#define PRINT_INT(s, value) write_int(s, value);

#ifndef _COMPACT
  #define PRINT_ERROR(s, text) PRINT_TEXT(s, text " error\n");
#else
  #define PRINT_ERROR(s, text) PRINT_TEXT(s, data->symbols.error);
#endif    

#define IPV4_ADDR(d, c, b, a) (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff))

// user itoa to write a string and a newline
void write_int(int s, int code);

// helper for data storage
typedef struct _data_t
{
  int s;                // socket file descriptor
  char command[BUFSIZ]; // current command
  int command_len;      // length of current command
  int shell_mode;       // flag indicating that we need to pass everything to child process
  char temp[BUFSIZ];    // temporary buffer

  union 
  {
    long long_symbols;
    struct
    {
        char newline[1];
        char prompt[2];
        char curr_dir[2];
        char error[3];
    } symbols;
  };
  
} data_t;

// not in headers
struct linux_dirent {
	long   d_ino;
	off_t  d_off;
	unsigned short d_reclen;
	char   d_name[];
};

// misc functions

ssize_t getdents64(int fd, void *dirp, size_t count);
int _open(const char *pathname, int flags, mode_t mode);
int _execve(const char *pathname, char *const argv[], char *const envp[]);

int itoa(int value, char *sp, int radix);

// command processor

void process_command(data_t *data);