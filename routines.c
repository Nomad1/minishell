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
// following routines pilfered from ryan "elfmaster" o'neill
// Nomad: signatures modified to match libc
int _open(const char *pathname, int flags, mode_t mode) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov %2, %%rdx\n"
        "mov $2, %%rax\n"
        "syscall" : : "g"(pathname), "g"(flags), "g"(mode));

      asm ("mov %%rax, %0" : "=r"(ret));              
      return ret;
}

int _close(int fd) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov $3, %%rax\n"
      "syscall" : : "g"(fd));
      
      asm("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}

ssize_t _read(int fd, void *buf, size_t count) {
    long ret;
     
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov %1, %%rsi\n"
      "mov %2, %%rdx\n"
      "mov $0, %%rax\n"
      "syscall" : : "g"(fd), "g"(buf), "g"(count));

    asm("mov %%rax, %0" : "=r"(ret));
    return (ssize_t)ret;
}

ssize_t _write(int fd, const void *buf, size_t count) {
    long ret;
     
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov %1, %%rsi\n"
      "mov %2, %%rdx\n"
      "mov $1, %%rax\n"
      "syscall" : : "g"(fd), "g"(buf), "g"(count));

    asm("mov %%rax, %0" : "=r"(ret));
    return (ssize_t)ret;
}

int _fstat(int fd, struct stat *buf) {
    long ret;
    
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov %1, %%rsi\n"
      "mov $5, %%rax\n"
      "syscall" : : "g"(fd), "g"(buf));

    asm("mov %%rax, %0" : "=r"(ret));
    return (int)ret;
}

int _dup2(int oldfd, int newfd) {
    long ret;
    
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov %1, %%rsi\n"
      "mov $33, %%rax\n"
      "syscall" : : "g"(oldfd), "g"(newfd));

    asm("mov %%rax, %0" : "=r"(ret));
    return (int)ret;
}

void *_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    long mmap_fd = fd;
    unsigned long mmap_off = offset;
    unsigned long mmap_flags = flags;
    unsigned long ret;

    __asm__ volatile(
     "mov %0, %%rdi\n"
     "mov %1, %%rsi\n"
     "mov %2, %%rdx\n"
     "mov %3, %%r10\n"
     "mov %4, %%r8\n"
     "mov %5, %%r9\n"
     "mov $9, %%rax\n"
     "syscall\n" : : "g"(addr), "g"(length), "g"(prot), "g"(mmap_flags), "g"(mmap_fd), "g"(mmap_off));

    asm ("mov %%rax, %0" : "=r"(ret));              
    return (void *)ret;
}

int _munmap(void *addr, size_t length) {
    long ret;
    
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov %1, %%rsi\n"
      "mov $11, %%rax\n"
      "syscall" : : "g"(addr), "g"(length));

    asm ("mov %%rax, %0" : "=r"(ret));
    return (int)ret;
}

int _kill(pid_t pid, int sig) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov $62, %%rax\n"
      "syscall" : : "g"(pid), "g"(sig));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}

pid_t _fork(void) {
      long ret;
      __asm__ volatile(
        "mov $57, %%rax\n"
      "syscall" : : );

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}

int _pipe(int pipefd[2]) {
    long ret;
    
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov $22, %%rax\n"
      "syscall" : : "g"(pipefd));

    asm("mov %%rax, %0" : "=r"(ret));
    return (int)ret;
}

// Sockets

int _shutdown(int sockfd, int how) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov $48, %%rax\n"
      "syscall" : : "g"(sockfd), "g"(how));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}

int _connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov %2, %%rdx\n"
        "mov $42, %%rax\n"
      "syscall" : : "g"(sockfd), "g"(addr), "g"(addrlen));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}


int _socket(int domain, int type, int protocol) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov %2, %%rdx\n"
        "mov $41, %%rax\n"
      "syscall" : : "g"(domain), "g"(type), "g"(protocol));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}

int _execve(const char *pathname, char *const argv[], char *const envp[]) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov %2, %%rdx\n"
        "mov $59, %%rax\n"
      "syscall" : : "g"(pathname), "g"(argv), "g"(envp));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}


u_int16_t _htons(u_int16_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char *s = (u_char *) &x;
	return (u_int16_t)(s[0] << 8 | s[1]);
#else
	return x;
#endif
}

int _epoll_create1(int flags) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov $291, %%rax\n"
      "syscall" : : "g"(flags));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}

int _epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov %2, %%rdx\n"
        "mov %3, %%r10\n"
        "mov $233, %%rax\n"
      "syscall" : : "g"(epfd), "g"(op), "g"(fd), "g"(event));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}

int _epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
      long ret;
      __asm__ volatile(
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov %2, %%rdx\n"
        "mov %3, %%r10\n"
        "mov $232, %%rax\n"
      "syscall" : : "g"(epfd), "g"(events), "g"(maxevents), "g"(timeout));

      asm ("mov %%rax, %0" : "=r"(ret));
      return (int)ret;
}