/*
 *  "$Id: l-bia.c,v 1.3 2008-07-09 20:32:39 br_lemes Exp $"
 *  Lua Built-In program (L-Bia)
 *  A self-running Lua interpreter. It turns your Lua program with all
 *  required modules and an interpreter into a single stand-alone program.
 *  Copyright (c) 2007,2008 Breno Ramalho Lemes
 *
 *  L-Bia comes with ABSOLUTELY NO WARRANTY; This is free software, and you
 *  are welcome to redistribute it under certain conditions; see COPYING
 *  for details.
 *
 *  <br_lemes@yahoo.com.br>
 *  http://l-bia.luaforge.net/
 */

#ifndef L_BIA_C
#define L_BIA_C

#include "lbconf.h"

lua_State *L = NULL;

typedef struct {
  char id[4];       /* LB02 */
  uint32_t dlen;    /* decompressed lenght */
  uint32_t adler32; /* adler32 checksum */
  uint32_t nlen;    /* compressed or normal lenght */
} lb_idtype;

void lb_error(const char *msg) {
  if (msg != NULL) fprintf(stderr,"ERROR: %s.\n",msg);
  lua_close(L);
  exit(EXIT_FAILURE);
}

void lb_cannot(const char *what, const char *name) {
  lua_pushfstring(L,"Cannot %s %s: %s",what,name,strerror(errno));
  lb_error(lua_tostring(L,-1));
}

int main(int argc, char *argv[]) {
  lb_idtype lb_id;
  lzo_uint  lb_overhead;
  lzo_uint  lb_offset;
  lzo_bytep lb_data;
  lzo_bytep lb_temp;

#ifdef _WIN32
  char selfname[MAX_PATH];
  if (GetModuleFileName(NULL,selfname,sizeof(selfname))==0)
    lb_error("Cannot locate this executable");
  argv[0] = selfname;
#endif

  if (lzo_init() != LZO_E_OK)
    lb_error("Internal LZO error");

  L = luaL_newstate();
  if (L == NULL) lb_error("Not enough memory");

  luaL_openlibs(L);

//  LBCONF_USERFUNC_INIT(L);

  /* open and load */
  FILE *lb_file = fopen(argv[0],"rb");
  if (lb_file == NULL)
    lb_cannot("open",argv[0]);
  if (fseek(lb_file,-sizeof(lb_id),SEEK_END)!=0)
    lb_cannot("seek",argv[0]);
  if (fread(&lb_id,sizeof(lb_id),1,lb_file)!=1)
    lb_cannot("read",argv[0]);
  if (memcmp(lb_id.id,"LB02",4)!=0)
    lb_error("Missing overlay");
  if (fseek(lb_file,-(sizeof(lb_id)+lb_id.nlen),SEEK_END)!=0)
    lb_cannot("seek",argv[0]);
  if (lb_id.dlen != 0) {
    lb_overhead = lb_id.dlen / 16 + 64 + 3;
    lb_offset = lb_id.dlen + lb_overhead - lb_id.nlen;
    lb_temp = (lzo_bytep)alloca(lb_id.dlen + lb_overhead);
    lb_data = lb_temp + lb_offset;
  } else
    lb_data = (lzo_bytep)alloca(lb_id.nlen);
  if (fread(lb_data,lb_id.nlen,1,lb_file)!=1)
    lb_cannot("read",argv[0]);
  fclose(lb_file);

  /* checksum */
  if (lzo_adler32(0,(lzo_bytep)lb_data,lb_id.nlen)!=lb_id.adler32)
    lb_error("Bad checksum");

  /* decompress */
  if (lb_id.dlen != 0) {
    lzo_uint new_len;
    int r = lzo1x_decompress(lb_data,lb_id.nlen,lb_temp,&new_len,NULL);
    if (r != LZO_E_OK || new_len != lb_id.dlen)
      lb_error("Overlapping decompression failed");
    lb_data = lb_temp;
  } else 
    lb_id.dlen = lb_id.nlen;

  char *buf = lb_data;
  uint32_t ptr = 0;
  do {
    char  _type = *buf++;
    char  _nlen = *buf++;
    char *_name = alloca((size_t)_nlen+1);
    memcpy(_name,buf,_nlen); _name[_nlen] = '\0'; buf += _nlen;
    uint32_t _size; memcpy(&_size,buf,4); buf += 4;
    buf += _size;
    ptr += 2 + _nlen + 4 + _size;
    printf("type: %d\nnlen: %d\nname: %s\n",_type,_nlen,_name);
  } while (ptr < lb_id.dlen);

//  LBCONF_USERFUNC_DONE(L);
  lua_close(L);
  return 0;
}

#endif
