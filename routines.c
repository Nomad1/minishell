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

void *get_proc_address(void *module, const char *name);

void *get_proc_address2(void *module, uint32_t hash);      // using base address
void *get_proc_address3(const char *path, uint32_t hash);  // using file path

void *get_module_handle(const char *module);
void *get_module_handle1(const char *module);
void *get_module_handle2(const char *module);

void *get_base(void);
void *get_base2(void);
void *get_base3(void);

Elf64_Phdr *elf_get_phdr(void *base, int type);
Elf64_Dyn *elf_get_dyn(void *base, int tag);

uint32_t gnu_hash(const uint8_t *name);

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

// return pointer to program header
Elf64_Phdr *elf_get_phdr(void *base, int type) {
    int        i;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    
    // sanity check on base and type
    if(base == NULL || type == PT_NULL) return NULL;
    
    // ensure this some semblance of ELF header
    if(*(uint32_t*)base != 0x464c457fUL) return NULL;
    
    // ok get offset to the program headers
    ehdr=(Elf64_Ehdr*)base;
    phdr=(Elf64_Phdr*)(base + ehdr->e_phoff);
    
    // search through list to find requested type
    for(i=0; i<ehdr->e_phnum; i++) {
      // if found
      if(phdr[i].p_type == type) {
        // return pointer to it
        return &phdr[i];
      }
    }
    // return NULL if not found
    return NULL;
}

uint64_t elf_get_delta(void *base) {
    Elf64_Phdr *phdr;
    uint64_t   low;
    
    // get pointer to PT_LOAD header
    // first should be executable
    phdr = elf_get_phdr(base, PT_LOAD);
    
    if(phdr != NULL) {
      low = phdr->p_vaddr;
    }
    return (uint64_t)base - low;
}

// return pointer to first dynamic type found
Elf64_Dyn *elf_get_dyn(void *base, int tag) {
    Elf64_Phdr *dynamic;
    Elf64_Dyn  *entry;
    
    // 1. obtain pointer to DYNAMIC program header
    dynamic = elf_get_phdr(base, PT_DYNAMIC);

    if(dynamic != NULL) {
      entry = (Elf64_Dyn*)(dynamic->p_vaddr + elf_get_delta(base));
      // 2. obtain pointer to type
      while(entry->d_tag != DT_NULL) {
        if(entry->d_tag == tag) {
          return entry;
        }
        entry++;
      }
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

void *get_module_handle2(const char *module) {
    Elf64_Phdr      *phdr;
    Elf64_Dyn       *got;
    void            *addr=NULL, *base;
    uint64_t        *ptrs;
    struct link_map *map;
    
    // 1. get the base of host ELF
    base = get_base();
    // 2. obtain pointer to dynamic program header
    phdr = (Elf64_Phdr*)elf_get_phdr(base, PT_DYNAMIC);
    
    if(phdr != NULL) {
      // 3. obtain global offset table
      got = elf_get_dyn(base, DT_PLTGOT);
      if(got != NULL) {
        ptrs = (uint64_t*)got->d_un.d_ptr;
        map   = (struct link_map *)ptrs[1];
        // 4. search through link_map for module
        while (map != NULL) {
          // 5 if no module provided, return first in the list
          if(module == NULL) {
            addr = (void*)map->l_addr;
            break;
          // otherwise, check by name
          } else if(_strstr(map->l_name, module)) {
            addr = (void*)map->l_addr;
            break;
          }
          map = (struct link_map *)map->l_next;
        }
      }
    }
    return addr;
}

uint32_t gnu_hash(const uint8_t *name) {
    uint32_t h = 5381;

    for(; *name; name++) {
      h = (h << 5) + h + *name;
    }
    return h;
}

// lookup by hash using the base address of module
void *get_proc_address2(void *module, uint32_t hash) {
    char            *path=NULL;
    Elf64_Phdr      *phdr;
    Elf64_Dyn       *got;
    uint64_t        *ptrs, addr, *base;
    struct link_map *map;
    
    if(module == NULL) return NULL;
    
    // 1. obtain pointer to dynamic program header
    base = get_base();
    phdr = (Elf64_Phdr*)elf_get_phdr(base, PT_DYNAMIC);
    
    if(phdr != NULL) {
      // 2. obtain global offset table
      got = elf_get_dyn(base, DT_PLTGOT);
      if(got != NULL) {
        ptrs = (uint64_t*)got->d_un.d_ptr;
        map   = (struct link_map *)ptrs[1];
        // 3. search through link_map for module
        while (map != NULL) {
          // this our module?
          if(map->l_addr == (uint64_t)module) {
            path = map->l_name;
            break;
          }
          map = (struct link_map *)map->l_next;
        }
      }
    }
    // not found? exit
    if(path == NULL) return NULL;
    addr = (uint64_t)get_proc_address3(path, hash);
    
    return (void*)((uint64_t)module + addr); 
}

// lookup by hash using the path of library (static lookup)
void* get_proc_address3(const char *path, uint32_t hash) {
    int         i, fd, cnt=0;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Shdr *shdr;
    Elf64_Sym  *syms=0;
    void       *addr=NULL;
    char       *strs=0;
    uint8_t    *map;
    struct stat fs;
    int         str[8];
    
    // /proc/self/exe
    str[0] = 0x6f72702f;
    str[1] = 0x65732f63;
    str[2] = 0x652f666c;
    str[3] = 0x00006578;

    // open file
    fd = _open(path == NULL ? (char*)str : path, O_RDONLY, 0);
    if(fd == 0) return NULL;
    // get the size
    if(_fstat(fd, &fs) == 0) {
      // map into memory
      map = (uint8_t*)_mmap(NULL, fs.st_size,  
        PROT_READ, MAP_PRIVATE, fd, 0);
      if(map != NULL) {
        ehdr = (Elf64_Ehdr*)map;
        shdr = (Elf64_Shdr*)(map + ehdr->e_shoff);
        // locate static or dynamic symbol table
        for(i=0; i<ehdr->e_shnum; i++) {
          if(shdr[i].sh_type == SHT_SYMTAB ||
             shdr[i].sh_type == SHT_DYNSYM) {
            strs = (char*)(map + shdr[shdr[i].sh_link].sh_offset);
            syms = (Elf64_Sym*)(map + shdr[i].sh_offset);
            cnt  = shdr[i].sh_size/sizeof(Elf64_Sym);
          }
        }
        // loop through string table for function
        for(i=0; i<cnt; i++) {
          // if found, save address
          if(gnu_hash(&strs[syms[i].st_name]) == hash) {
            addr = (void*)syms[i].st_value;
          }
        }
        _munmap(map, fs.st_size);
      }
    }
    _close(fd);
    return addr;
}

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

int fstat(int fd, struct stat *buf) {
    long ret;
    
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov %1, %%rsi\n"
      "mov $5, %%rax\n"
      "syscall" : : "g"(fd), "g"(buf));
    asm("mov %%rax, %0" : "=r"(ret));
    return (int)ret;
}

void *_mmap(void *addr, unsigned long len, 
  unsigned long prot, unsigned long flags, 
  long fd, unsigned long off) {
    long mmap_fd = fd;
    unsigned long mmap_off = off;
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
     "syscall\n" : : "g"(addr), "g"(len), "g"(prot), "g"(flags), "g"(mmap_fd), "g"(mmap_off));
    asm ("mov %%rax, %0" : "=r"(ret));              
    return (void *)ret;
}

int _munmap(void *addr, size_t len) {
    long ret;
    
    __asm__ volatile(
      "mov %0, %%rdi\n"
      "mov %1, %%rsi\n"
      "mov $11, %%rax\n"
      "syscall" :: "g"(addr), "g"(len));
    asm ("mov %%rax, %0" : "=r"(ret));
    return (int)ret;
}

void init_api(data_t *ds)
{
      void               *clib;
      int                str[8];

      // 1. resolve the address of _dl_addr in libc.so
      str[0] = 0x6362696c;
      str[1] = 0;
      clib = get_module_handle2((char*)str);
      
      // ds.api._dlopen = (dlopen_t)get_proc_address2(clib, 0xf2cb98a2 /* __libc_dlopen_mode */);
      
      ds->api._pipe          = get_proc_address2(clib, 0x7c9c4773);
      ds->api._fork          = get_proc_address2(clib, 0x7c96e577);
      ds->api._socket        = get_proc_address2(clib, 0x1c31032e);
      ds->api._htons         = get_proc_address2(clib, 0x0f9a7751);
      ds->api._connect       = get_proc_address2(clib, 0xd3764dcf);
      ds->api._dup2          = get_proc_address2(clib, 0x7c95e5c0);
      ds->api._close         = get_proc_address2(clib, 0x0f3b9a5b);
      ds->api._execve        = get_proc_address2(clib, 0xfc2c9fc5);
      ds->api._epoll_create1 = get_proc_address2(clib, 0x5694d925);
      ds->api._epoll_ctl     = get_proc_address2(clib, 0xa847e103);
      ds->api._epoll_wait    = get_proc_address2(clib, 0xb14ea835);
      ds->api._open          = get_proc_address2(clib, 0x7c9bd777);
      ds->api._write         = get_proc_address2(clib, 0x10a8b550);
      ds->api._read          = get_proc_address2(clib, 0x7c9d4d41);
      ds->api._shutdown      = get_proc_address2(clib, 0xfc460361);
      ds->api._kill          = get_proc_address2(clib, 0x7c998911);
}