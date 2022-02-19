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
  
#include "include.h"

#ifdef EXT_FUNC
int compare(const char *s1, const char *s2) {
    while(*s1 && *s2) {
      if(*s1 != *s2) {
        return 0;
      }
      s1++; s2++;
    }
    return *s2 == 0;
}

const char* _strstr(const char *s1, const char *s2) {
    while (*s1) {
      if((*s1 == *s2) && compare(s1, s2)) return s1;
      s1++;
    }
    return NULL;
}

uint64_t hex2bin(const char hex[]) {
    uint64_t r=0;
    char     c;
    int      i;
    
    for(i=0; i<16; i++) {
      c = hex[i];
      if(c >= '0' && c <= '9') { 
        c = c - '0';
      } else if(c >= 'a' && c <= 'f') {
        c = c - 'a' + 10;
      } else if(c >= 'A' && c <= 'F') {
        c = c - 'A' + 10;
      } else break;
      r *= 16;
      r += c;
    }
    return r;
}

void *get_base(void) {
    int  maps;
    void *addr;
    char line[32];
    int  str[8];
    
    // /proc/self/maps
    str[0] = 0x6f72702f;
    str[1] = 0x65732f63;
    str[2] = 0x6d2f666c;
    str[3] = 0x00737061;
    str[4] = 0;
    
    maps = _open((char*)str, O_RDONLY, 0);
    if(!maps) return NULL;
    
    _read(maps, line, 16);
    _close(maps);
    
    addr = (void*)hex2bin(line);
    return addr;
}
#endif

size_t strlen(const char *s) {
  size_t i = 0;
  while(*s++)
    i++;
  return i;
}

// following routines pilfered from ryan "elfmaster" o'neill
// Nomad: signatures modified to match libc
int _open(const char *pathname, int flags, mode_t mode) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        // "mov %2, %%rdx\n"
        "mov $2, %%eax\n"
        "syscall" ::);// "g"((unsigned long)pathname), "g"((unsigned long)flags), "g"((unsigned long)mode));

      // asm ("mov %%rax, %0" : "=r"(ret));              
      // return ret;
}

int close(int fd) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        "mov $3, %%eax\n"
      "syscall" ::);// "g"((unsigned long)fd));
      
      // asm("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

int uname(struct utsname *buf) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        "mov $63, %%eax\n"
      "syscall" ::);// "g"((unsigned long)buf));
      
      // asm("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

ssize_t read(int fd, void *buf, size_t count) {
    // long ret;
     
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      // "mov %2, %%rdx\n"
      "mov $0, %%eax\n"
      "syscall" ::);// "g"((unsigned long)fd), "g"((unsigned long)buf), "g"((unsigned long)count));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    // long ret;
     
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      // "mov %2, %%rdx\n"
      "mov $1, %%eax\n"
      "syscall" ::);// "g"((unsigned long)fd), "g"((unsigned long)buf), "g"((unsigned long)count));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (ssize_t)ret;
}

char * getcwd (char *buffer, size_t size) {
    // long ret;
     
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      "mov $79, %%eax\n"
      "syscall" ::);// "g"((unsigned long)buffer), "g"((unsigned long)size));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (char *)ret;
}

ssize_t getdents64(int fd, void *dirp, size_t count) {
    // long ret;
     
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      // "mov %2, %%rdx\n"
      "mov $217, %%eax\n"
      "syscall" ::);// "g"((unsigned long)fd), "g"((unsigned long)dirp), "g"((unsigned long)count));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (ssize_t)ret;
}

int fstat(int fd, struct stat *buf) {
    // long ret;
    
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      "mov $5, %%eax\n"
      "syscall" ::);// "g"((unsigned long)fd), "g"((unsigned long)buf));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (int)ret;
}

int stat(const char * filename, struct stat *buf) {
    // long ret;
    
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      "mov $5, %%eax\n"
      "syscall" ::);// "g"((unsigned long)filename), "g"((unsigned long)buf));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (int)ret;
}

int dup2(int oldfd, int newfd) {
    // long ret;
    
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      "mov $33, %%eax\n"
      "syscall" ::);// "g"((unsigned long)oldfd), "g"((unsigned long)newfd));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (int)ret;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    // long mmap_fd = fd;
    // unsigned long mmap_off = offset;
    // unsigned long mmap_flags = flags;
    // unsigned long ret;

    __asm__ volatile(
    //  "mov %0, %%rdi\n"
    //  "mov %1, %%rsi\n"
    //  "mov %2, %%rdx\n"
    //  "mov %3, %%r10\n"
    //  "mov %4, %%r8\n"
    //  "mov %5, %%r9\n"
     "mov %%rcx, %%r10\n"
     "mov $9, %%eax\n"
     "syscall\n" ::);// "g"((unsigned long)addr), "g"((unsigned long)length), "g"((unsigned long)prot), "g"((unsigned long)mmap_flags), "g"((unsigned long)mmap_fd), "g"((unsigned long)mmap_off));

    // asm ("mov %%rax, %0" : "=r"(ret));              
    // return (void *)ret;
}

int munmap(void *addr, size_t length) {
    // long ret;
    
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      // "mov %1, %%rsi\n"
      "mov $11, %%eax\n"
      "syscall" ::);// "g"((unsigned long)addr), "g"((unsigned long)length));

    // asm ("mov %%rax, %0" : "=r"(ret));
    // return (int)ret;
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz) {
    __asm__ volatile(
      "mov $89, %%eax\n"
      "syscall" ::);
}

int _execve(const char *pathname, char *const argv[], char *const envp[]) {
    __asm__ volatile(
      "mov $59, %%eax\n"
      "syscall" ::);
}

void _exit(int status) {
    __asm__ volatile(
      "mov $60, %%eax\n"
      "syscall"::);
}

int kill(pid_t pid, int sig) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        "mov $62, %%eax\n"
      "syscall" ::);// "g"((unsigned long)pid), "g"((unsigned long)sig));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

pid_t fork(void) {
      // long ret;
      __asm__ volatile(
        "mov $57, %%eax\n"
      "syscall" : : );

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

int pipe(int pipefd[2]) {
    // long ret;
    
    __asm__ volatile(
      // "mov %0, %%rdi\n"
      "mov $22, %%eax\n"
      "syscall" ::);// "g"((unsigned long)pipefd));

    // asm("mov %%rax, %0" : "=r"(ret));
    // return (int)ret;
}

// Sockets

int shutdown(int sockfd, int how) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        "mov $48, %%eax\n"
      "syscall" ::);// "g"((unsigned long)sockfd), "g"((unsigned long)how));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        // "mov %2, %%rdx\n"
        "mov $42, %%eax\n"
      "syscall" ::);// "g"((unsigned long)sockfd), "g"((unsigned long)addr), "g"((unsigned long)addrlen));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}


int socket(int domain, int type, int protocol) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        // "mov %2, %%rdx\n"
        "mov $41, %%eax\n"
      "syscall" ::);// "g"((unsigned long)domain), "g"((unsigned long)type), "g"((unsigned long)protocol));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
      /*__asm__ volatile(
        "mov $41, %%eax\n"
        "syscall" : "=r"(ret));
        return (int)ret;*/
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        // "mov %2, %%rdx\n"
        "mov $59, %%eax\n"
      "syscall" ::);// "g"((unsigned long)pathname), "g"((unsigned long)argv), "g"((unsigned long)envp));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

#ifndef htons
u_int16_t htons(u_int16_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char *s = (u_char *) &x;
	return (u_int16_t)(s[0] << 8 | s[1]);
#else
	return x;
#endif
}
#endif

inline int epoll_create1(int flags) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        "mov $291, %%eax\n"
      "syscall" ::);// "g"((unsigned long)flags));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

inline int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        // "mov %2, %%rdx\n"
        // "mov %3, %%r10\n"
        "mov %%rcx, %%r10\n"
        "mov $233, %%eax\n"
      "syscall" ::);// "g"((unsigned long)epfd), "g"((unsigned long)op), "g"((unsigned long)fd), "g"((unsigned long)event));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        // "mov %2, %%rdx\n"
        // "mov %3, %%r10\n"
        "mov %%rcx, %%r10\n"
        "mov $7, %%eax\n"
      "syscall" ::);// "g"((unsigned long)epfd), "g"((unsigned long)op), "g"((unsigned long)fd), "g"((unsigned long)event));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

inline int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
      // long ret;
      __asm__ volatile(
        // "mov %0, %%rdi\n"
        // "mov %1, %%rsi\n"
        // "mov %2, %%rdx\n"
        // "mov %3, %%r10\n"
        "mov %%rcx, %%r10\n"
        "mov $232, %%eax\n"
      "syscall" ::);// "g"((unsigned long)epfd), "g"((unsigned long)events), "g"((unsigned long)maxevents), "g"((unsigned long)timeout));

      // asm ("mov %%rax, %0" : "=r"(ret));
      // return (int)ret;
}

// Yet, another good itoa implementation
// returns: the length of the number string
int itoa(int value, char *sp, int radix)
{
    char tmp[16];// be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (radix == 10 && value < 0);    
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp)
    {
        i = v % radix;
        v /= radix;
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign) 
    {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
        *sp++ = *--tp;

    return len;
}

char * strcpy(char *to, const char *from)
{
	char *save = to;
	for (; (*to = *from) != '\0'; ++from, ++to);
	return(save);
}

void write_int(int s, int code)
{
  long itoabuf[2];
  int len;
  len = itoa(code, (char *)itoabuf, 10);
  ((char *)itoabuf)[len] = '\n';
  ((char*)itoabuf)[len + 1] = '0';
  write(s, (char *)itoabuf, len + 1);
}