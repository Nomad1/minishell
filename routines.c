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

#ifdef LIBC

int strcmp(const char *s1, const char *s2)
{
  while (*s1 && *s2)
  {
    if (*s1 != *s2)
    {
      return 0;
    }
    s1++;
    s2++;
  }
  return *s2 == 0;
}

char *strstr(const char *s1, const char *s2)
{
  while (*s1)
  {
    if ((*s1 == *s2) && strcmp(s1, s2))
      return (char *)s1;
    s1++;
  }
  return NULL;
}

long hex2bin(const char * hex)
{
  long r = 0;
  char c;
  int i;

  for (i = 0; i < 16; i++)
  {
    c = hex[i];
    if (c >= '0' && c <= '9')
    {
      c = c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
      c = c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F')
    {
      c = c - 'A' + 10;
    }
    else
      break;
    r = (r << 4) + (long)c;
  }
  return r;
}

long get_libc(char *buffer, int len)
{
  char maps_name[] = "/proc/self/maps";
  char lookup_string[] = "/libc-";
  int fd;
  long addr;
  int nread;
  char *pchunk = buffer;
  int chunk = len >> 1;
  int i;
  char *ptr;

  fd = _open(maps_name, O_RDONLY, 0);
  if (!fd)
  {
    // PRINT_TEXT(STDERR_FILENO, "Failed to open maps!\n");
    return 0;
  }

  for (;;)
  {
    nread = read(fd, pchunk, chunk);

    if (nread < 0)
    {
      // PRINT_TEXT(STDERR_FILENO, "Failed to read from maps!\n");
      close(fd);
      return 0;
    }

    if (nread != 0)
    {
      *(pchunk + nread) = 0;

      ptr = strstr(buffer, lookup_string);

      if (ptr)
      {
        // look for previous \n
        while(ptr != buffer && *(ptr-1)!= '\n')
          ptr--;

        addr = (long)hex2bin(ptr);

        break;
      }

      // move data to the beginning of the buffer
      for (i = 0; i < nread; i++)
        buffer[i] = *(pchunk + i);

      pchunk = buffer + nread;
    }
    else
      break;
  }

  close(fd);
  return addr;
}

char *strcpy(char *to, const char *from)
{
  char *save = to;
  for (; (*to = *from) != '\0'; ++from, ++to)
    ;
  return (save);
}

#endif

size_t strlen(const char *s)
{
  size_t i = 0;
  while (*s++)
    i++;
  return i;
}

int _open(const char *pathname, int flags, mode_t mode)
{
  __asm__ volatile(
      "mov $2, %%eax\n"
      "syscall" ::);
}

int close(int fd)
{
  __asm__ volatile(
      "mov $3, %%eax\n"
      "syscall" ::);
}

int uname(struct utsname *buf)
{
  __asm__ volatile(
      "mov $63, %%eax\n"
      "syscall" ::);
}

ssize_t read(int fd, void *buf, size_t count)
{
  __asm__ volatile(
      "mov $0, %%eax\n"
      "syscall" ::);
}

ssize_t write(int fd, const void *buf, size_t count)
{
  __asm__ volatile(
      "mov $1, %%eax\n"
      "syscall" ::);
}

char *getcwd(char *buffer, size_t size)
{
  __asm__ volatile(
      "mov $79, %%eax\n"
      "syscall" ::);
}

ssize_t getdents64(int fd, void *dirp, size_t count)
{
  __asm__ volatile(
      "mov $217, %%eax\n"
      "syscall" ::);
}

int _fstat(int fd, struct stat *buf)
{
  __asm__ volatile(
      "mov $5, %%eax\n"
      "syscall" ::);
}

int stat(const char *filename, struct stat *buf)
{
  __asm__ volatile(
      "mov $5, %%eax\n"
      "syscall" ::);
}

int dup2(int oldfd, int newfd)
{
  __asm__ volatile(
      "mov $33, %%eax\n"
      "syscall" ::);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  __asm__ volatile(
      "mov %%rcx, %%r10\n"
      "mov $9, %%eax\n"
      "syscall\n" ::);
}

int munmap(void *addr, size_t length)
{
  __asm__ volatile(
      "mov $11, %%eax\n"
      "syscall" ::);
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
  __asm__ volatile(
      "mov $89, %%eax\n"
      "syscall" ::);
}

int _execve(const char *pathname, char *const argv[], char *const envp[])
{
  __asm__ volatile(
      "mov $59, %%eax\n"
      "syscall" ::);
}

void _exit(int status)
{
  __asm__ volatile(
      "mov $60, %%eax\n"
      "syscall" ::);
}

int kill(pid_t pid, int sig)
{
  __asm__ volatile(
      "mov $62, %%eax\n"
      "syscall" ::);
}

pid_t fork(void)
{
  __asm__ volatile(
      "mov $57, %%eax\n"
      "syscall"
      ::);
}

int pipe(int pipefd[2])
{
  __asm__ volatile(
      "mov $22, %%eax\n"
      "syscall" ::);
}

// Sockets

int shutdown(int sockfd, int how)
{
  __asm__ volatile(
      "mov $48, %%eax\n"
      "syscall" ::);
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  __asm__ volatile(
      "mov $42, %%eax\n"
      "syscall" ::);
}

int socket(int domain, int type, int protocol)
{
  __asm__ volatile(
      "mov $41, %%eax\n"
      "syscall" ::);
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
  __asm__ volatile(
      "mov $59, %%eax\n"
      "syscall" ::);
}

#ifndef htons
u_int16_t htons(u_int16_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  u_char *s = (u_char *)&x;
  return (u_int16_t)(s[0] << 8 | s[1]);
#else
  return x;
#endif
}
#endif

#ifdef EPOLL
inline int epoll_create1(int flags)
{
  __asm__ volatile(
      "mov $291, %%eax\n"
      "syscall" ::);
}

inline int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
  __asm__ volatile(
      "mov %%rcx, %%r10\n"
      "mov $233, %%eax\n"
      "syscall" ::);
}

inline int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
  __asm__ volatile(
      "mov %%rcx, %%r10\n"
      "mov $232, %%eax\n"
      "syscall" ::);
}
#endif

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  __asm__ volatile(
      "mov %%rcx, %%r10\n"
      "mov $7, %%eax\n"
      "syscall" ::);
}

// Yet, another good itoa implementation
// returns: the length of the number string
int _ltoa(long value, char *sp, int radix)
{
  char tmp[16]; // be careful with the length of the buffer
  char *tp = tmp;
  long i;
  unsigned long v;

  int sign = (radix == 10 && value < 0);
  if (radix == 10 && value < 0)
    v = -value;
  else
    v = (unsigned long)value;

  while (v || tp == tmp)
  {
    i = v % radix;
    v /= radix;
    if (i < 10)
      *tp++ = i + '0';
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
/*
long clone(unsigned long flags, void *child_stack,
          void *ptid, void *ctid,
          struct pt_regs *regs)
{
}*/
