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

// Libc functions implemented as asm syscalls

// files
int _open(const char *pathname, int flags, mode_t mode);
int _close(int fd);
ssize_t _read(int fd, void *buf, size_t count);
ssize_t _write(int fd, const void *buf, size_t count);

int _fstat(int fd, struct stat *buf);

// pipes/processes
int _pipe(int pipefd[2]);
pid_t _fork(void);
int _kill(pid_t pid, int sig);
int _dup2(int oldfd, int newfd);

// mmap

void *_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int _munmap(void *addr, size_t length);

// execve

int _execve(const char *pathname, char *const argv[], char *const envp[]);

// sockets
int _shutdown(int sockfd, int how);
int _connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int _socket(int domain, int type, int protocol);
u_int16_t _htons(u_int16_t x);

// epoll sockets

int _epoll_create1(int flags);
int _epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int _epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);