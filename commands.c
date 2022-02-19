#include "include.h"

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
  PRINT_TEXT(data->s, data->symbols.newline);
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
  int res;
  char *args[2];

  args[0] = (char *)path;
  args[1] = 0;

  res = _execve(path, args, NULL);

  if (res < 0)
  {
#ifndef _COMPACT
    PRINT_TEXT(data->s, "exec() error: ");
    PRINT_INT(data->s, res);
#else
    PRINT_TEXT(data->s, data->symbols.error);
#endif
  }
}

void ls_command(data_t *data, const char *path)
{
  int fd, nread, bpos, count, len;
  struct linux_dirent *d;
  char d_type;

  fd = _open(path, 0 /*O_RDONLY  | O_DIRECTORY*/, 0);
  if (fd < 0)
  {
    PRINT_ERROR(data->s, "open()");
    return;
  }

#ifndef _COMPACT
  PRINT_TEXT(data->s, "- Listing ");
  PRINT_STR(data->s, path);
  PRINT_TEXT(data->s, data->symbols.newline);
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
      PRINT_TEXT(data->s, data->symbols.newline);

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
  PRINT_TEXT(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->nodename);
  PRINT_TEXT(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->release);
  PRINT_TEXT(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->version);
  PRINT_TEXT(data->s, data->symbols.newline);
  PRINT_STR(data->s, udata->machine);
  PRINT_TEXT(data->s, data->symbols.newline);
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
  PRINT_TEXT(data->s, data->symbols.newline);
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
    readlink_command(data, "/proc/self/cwd");
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
    PRINT_TEXT(data->s, data->symbols.error);
#endif
}
