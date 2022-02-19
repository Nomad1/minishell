#include "include.h"

void cat_command(data_t *data, const char *path)
{
  int fd, nread, bpos, count, len;
  char d_type;
  long newline[1];
  newline[0] = '\n';

  fd = _open(path, O_RDONLY, 0);
  if (fd < 0)
  {
    PRINT_TEXT(data->s, "open() error!\n");
    return;
  }

  PRINT_TEXT(data->s, "- Listing file ");
  write(data->s, path, strlen(path));
  PRINT_TEXT(data->s, "\n");

  count = 0;

  for (;;)
  {
    nread = read(fd, data->temp, BUFSIZ);
    if (nread < 0)
    {
      close(fd);
      PRINT_TEXT(data->s, "read() error!\n");
      return;
    }

    if (nread != 0)
    {
      write(data->s, data->temp, nread);
      count += nread;
    }
    else
      break;
  }

  close(fd);

  PRINT_TEXT(data->s, "\n- Total: ");
  {
    long itoabuf[1];
    itoabuf[0] = 0;
    itoa(count, (char *)itoabuf, 10);
    write(data->s, (char *)itoabuf, strlen((char *)itoabuf));
  }
  PRINT_TEXT(data->s, " bytes\n");
}

void exec_command(data_t *data, const char *path)
{
  int res;
  char *args[2];

  args[0] = (char*)path;
  args[1] = 0;

  res = _execve(path, args, NULL);

  if (res < 0)
  {
    ERROR_TEXT(data->s, "exec() error: ", res);
    return;
  }
}

void ls_command(data_t *data, const char *path)
{
  int fd, nread, bpos, count, len;
  struct linux_dirent *d;
  char d_type;
  long newline[1];
  long itoabuf[1];
  newline[0] = '\n';

  fd = _open(path, O_RDONLY /* | O_DIRECTORY*/, 0);
  if (fd < 0)
  {
    PRINT_TEXT(data->s, "open() error!\n");
    return;
  }

  PRINT_TEXT(data->s, "- Listing ");
  write(data->s, path, strlen(path));
  PRINT_TEXT(data->s, "\n");

  count = 0;

  // for (;;)
  {
    nread = getdents64(fd, data->temp, BUFSIZ);
    if (nread < 0)
    {
      close(fd);
      ERROR_TEXT(data->s, "getdents64() error: ", nread);
      return;
    }

    if (nread > 0)
    {
      for (bpos = 0; bpos < nread;)
      {
        d = (struct linux_dirent *)(data->temp + bpos);
        d_type = *(data->temp + bpos + d->d_reclen - 1);
        bpos += d->d_reclen;
        len = strlen(d->d_name);

        if (len && d->d_name[0] != ' ')
        {
          write(data->s, d->d_name, len);

          count++;

          write(data->s, (char *)newline, 1);
        }
      }
    }
  }

  close(fd);

  PRINT_TEXT(data->s, "- Total: ");
  {
    itoabuf[0] = 0;
    itoa(count, (char *)itoabuf, 10);
    write(data->s, (char *)itoabuf, strlen((char *)itoabuf));
  }
  PRINT_TEXT(data->s, " entries\n");
}

void uname_command(data_t *data)
{
  long newline[1];
  struct utsname udata;
  int res = uname(&udata);

  if (res < 0)
  {
    ERROR_TEXT(data->s, "uname() error: ", res);    
    return;
  }

  newline[0] = '\n';

  write(data->s, udata.sysname, strlen(udata.sysname));
  write(data->s, (char *)newline, 1);
  write(data->s, udata.nodename, strlen(udata.nodename));
  write(data->s, (char *)newline, 1);
  write(data->s, udata.release, strlen(udata.release));
  write(data->s, (char *)newline, 1);
  write(data->s, udata.version, strlen(udata.version));
  write(data->s, (char *)newline, 1);
  write(data->s, udata.machine, strlen(udata.machine));
  write(data->s, (char *)newline, 1);
}

void pwd_command(data_t *data)
{
  int len;

  // char *s = getcwd(data->temp, BUFSIZ);
  len = readlink("/proc/self/fd/dirfd" /*"/proc/self/exe"*/, data->temp, BUFSIZ - 1);

  if (len < 0)
  {
    ERROR_TEXT(data->s, "readlink() error: ", len);
    return;
  }

  write(data->s, data->temp, strlen(data->temp));

  // str[0] = TEXT('\n');
  // write(data->s, (char *)str, strlen((char*)str));
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
    else if (len > 3 && data->command[1] == 's' && data->command[2] == ' ')
      ls_command(data, data->command + 3);
    else
    {
      char current[2] = ".";
      ls_command(data, current);
    }
  }
  else if (data->command[0] == 'u') // uname
    uname_command(data);
  else if (data->command[0] == 'p') // pwd
    pwd_command(data);
  else if (data->command[0] == 'c') // cat or cd
  {
    if (len > 2 && data->command[1] == ' ')
      cat_command(data, data->command + 2);
    else if (len > 4 && data->command[1] == 'a' && data->command[2] == 't' && data->command[3] == ' ')
      cat_command(data, data->command + 4);
    else
      PRINT_TEXT(data->s, "Syntax: cat <file>\n");
  }
  else if (data->command[0] == 'e') // exec
  {
    if (len > 2 && data->command[1] == ' ')
      exec_command(data, data->command + 2);
    else if (len > 5 && data->command[1] == 'x' && data->command[2] == 'e' && data->command[3] == 'c' && data->command[4] == ' ')
      exec_command(data, data->command + 5);
    else
      PRINT_TEXT(data->s, "Syntax: exec <file>\n");
  }
  else if (data->command[0] == 'q') // quit
    exit(0);
  else
    PRINT_TEXT(data->s, "Unknown command!\n");
}
