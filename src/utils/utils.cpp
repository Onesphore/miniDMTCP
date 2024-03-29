#include <utils/utils.hpp>

int 
_readline(int fd, char *line)
{
  memset(line, 0, 128);

  char c;
  char *line_p = line;
  int cnt;
  while ((cnt = read(fd, &c, 1)) !=0)
  {
    *line_p++ = c;
    if (c == '\n'){
      break;
    }
  }
  
  if (cnt == 0){ // end of file read() return value = 0.
    return -1;
  }

  return 0;
}


// FIXME: do we need 'offset'
void 
fill_memMap(memMap_t* memMap_ptr, char *line, off_t offset)
{
  memset(memMap_ptr, 0, sizeof(memMap_t));
  char *line_p = line;
  //1. get the address where this memory section begins
  char *addr_begin, *addr_end;
  char hex_str[17];
  char *hex_str_p = hex_str;

  memset(hex_str, 0, 17);
  while (*line_p != '-')
  {
    *hex_str_p++ = *line_p++;
  }

  addr_begin = hexstring_to_int(hex_str);

  //  . get the address where this memory section ends
  hex_str_p = hex_str;
  memset(hex_str, 0, 17);
  line_p++; // to get past "-"
  while (*line_p != ' ')
  {
    *hex_str_p++ = *line_p++;
  }
  addr_end = hexstring_to_int(hex_str);
 
  memMap_ptr->vaddr = addr_begin;
  memMap_ptr->data_size = (size_t) (addr_end - addr_begin);
 
  line_p++; // get past " "

  //3. get access mode: r/w/x
  char permissions = 0;
  if (*line_p++ = 'r') {
    permissions = permissions | 1;
  }
  if (*line_p++ = 'w') {
    permissions = permissions | (1 << 1);
  }
  if (*line_p++ = 'x') {
    permissions = permissions | (1 << 2);
  }
  memMap_ptr->permissions = permissions;
//   memMap_ptr->readable = *line_p++;
//   memMap_ptr->writable = *line_p++;
//   memMap_ptr->executable = *line_p++;

  //4. check if this memory section is a stack region
  while (*line_p != 's' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
  {
    memMap_ptr->is_stack = false;
    return;
  }  


  // FIXME: not really robust
  if (*line_p == 's')
  {
    char stack_str[6];
    char *stack_str_p = stack_str;
    int i;
    for (i=0; i<5; ++i)
    {
      *stack_str_p++ = *line_p++;
    }
    *stack_str_p = 0;
    if (!strcmp(stack_str, "stack")) {
      memMap_ptr->is_stack    = true;
      memMap_ptr->data_offset = offset;
    }
    
    return;
  }

  memMap_ptr->is_stack    = false;
  memMap_ptr->data_offset = offset;
  return;
}

void ckpt_memMaps() {
 ;
}

char* 
hexstring_to_int(char *hexstring)
{
  char* ret_val=0;

  int base = 16; 
  unsigned long long base_to_exp;

  size_t len = strlen(hexstring);
  int exp;
  for (exp=len-1; exp>=0; --exp)
  {
    if (exp == len-1){
      base_to_exp = 1;
    }         
    else{
      base_to_exp *= 16;
    }

    if (hexstring[exp] >= 'a' && hexstring[exp] <= 'f'){
      ret_val += (hexstring[exp] - ('a'-10)) * base_to_exp;
    } else {
      ret_val += (hexstring[exp] -'0') * base_to_exp;
    }
  }
  return ret_val;
}

int 
is_stack_line(char *line)
{
  char *line_p = line;
  while (*line_p != 's' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 's')
  {
    char stack_str[6];
    char *stack_str_p = stack_str;
    int i;
    for (i=0; i<5; ++i)
    {
      *stack_str_p++ = *line_p++;
    }
    *stack_str_p = 0;
    if (!strcmp(stack_str, "stack"))
      return 1;
  }

  return 0;
}

int 
is_vvar_line(char *line)
{
  char *line_p = line;
  while (*line_p != 'v' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 'v')
  {
    char vvar_str[5];
    char *vvar_str_p = vvar_str;
    int i;
    for (i=0; i<4; ++i)
    {
      *vvar_str_p++ = *line_p++;
    }
    *vvar_str_p = 0;
    if (!strcmp(vvar_str, "vvar"))
      return 1;
  }

  return 0;
}

int 
is_vdso_line(char *line)
{
  char *line_p = line;
  while (*line_p != 'v' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 'v')
  {
    char vdso_str[5];
    char *vdso_str_p = vdso_str;
    int i;
    for (i=0; i<4; ++i)
    {
      *vdso_str_p++ = *line_p++;
    }
    *vdso_str_p = 0;
    if (!strcmp(vdso_str, "vdso"))
      return 1;
  }

  return 0;
}

int 
is_vsyscall_line(char *line)
{
  char *line_p = line;
  while (*line_p != 'v' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 'v')
  {
    char vsyscall_str[9];
    char *vsyscall_str_p = vsyscall_str;
    int i;
    for (i=0; i<8; ++i)
    {
      *vsyscall_str_p++ = *line_p++;
    }
    *vsyscall_str_p = 0;
    if (!strcmp(vsyscall_str, "vsyscall"))
      return 1;
  }

  return 0;
}

bool
is_skip_region(char *line) {
   char *line_p = line;
  while (*line_p != ' ' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return false;

  if (*(line_p+1) == '-' && *(line_p+2) == '-' && *(line_p+3) == '-')
  {
    return true;
  }

  return false;
}
