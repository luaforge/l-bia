/*
 *  "$Id: l-bia.c,v 1.2 2008-06-26 23:38:16 br_lemes Exp $"
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

#ifdef _WIN32
#include <windows.h>
#endif

#include "lbconf.h"

#define LB_FLATMD 101
#define LB_MLZOMD 102
#define LB_FLATID "Flat"
#define LB_MLZOID "mLZO"

lua_State *L = NULL;

typedef struct {
  char open;     /* <                   */
  char sign[6];  /* L-Bia#              */
  char mode[4];  /* Flat|mLZO           */
  char sep1;     /* :                   */
  char nlen[10]; /* normal lenght       */
  char sep2;     /* :                   */
  char dlen[10]; /* decompressed lenght */
  char close;    /* >                   */
} lb_idtype;

void lb_error(const char *msg) {
  if (msg != NULL) fprintf(stderr,"ERROR: %s.\n",msg);
  lua_close(L);
  exit(EXIT_FAILURE);
}

void lb_cannot(const char *what,const char *name) {
  lua_pushfstring(L,"Cannot %s %s: %s",what,name,strerror(errno));
  lb_error(lua_tostring(L,-1));
}

static const luaL_Reg luserlibs[] = {
#ifdef LBCONF_LIB_BASE
  {"",luaopen_base},
#endif
#ifdef LBCONF_LIB_LOAD
  {LUA_LOADLIBNAME,luaopen_package},
#endif
#ifdef LBCONF_LIB_TABLE
  {LUA_TABLIBNAME,luaopen_table},
#endif
#ifdef LBCONF_LIB_IO
  {LUA_IOLIBNAME,luaopen_io},
#endif
#ifdef LBCONF_LIB_OS
  {LUA_OSLIBNAME,luaopen_os},
#endif
#ifdef LBCONF_LIB_STRING
  {LUA_STRLIBNAME,luaopen_string},
#endif
#ifdef LBCONF_LIB_MATH
  {LUA_MATHLIBNAME,luaopen_math},
#endif
#ifdef LBCONF_LIB_DB
  {LUA_DBLIBNAME,luaopen_debug},
#endif
  {NULL,NULL}
};

LUALIB_API void luaL_openuserlibs(lua_State *L) {
  const luaL_Reg *lib = luserlibs;
  for (;lib->func;lib++) {
    lua_pushcfunction(L,lib->func);
    lua_pushstring(L,lib->name);
    lua_call(L,1,0);
  }
}

int main(int argc, char *argv[]){
  int lb_nlen, lb_dlen, lb_mode;
  lb_idtype lb_id;

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

/*luaL_openlibs(L); */
  luaL_openuserlibs(L);

  LBCONF_USERFUNC_INIT(L);

  /* extracting pass */ 
  FILE *lb_file = fopen(argv[0],"rb");
  if (lb_file == NULL)
    lb_cannot("open",argv[0]);
  if (fseek(lb_file,-sizeof(lb_id),SEEK_END)!=0)
    lb_cannot("seek",argv[0]);
  if (fread(&lb_id,sizeof(lb_id),1,lb_file)!=1)
    lb_cannot("read",argv[0]);

  /* mode detect subpass */
  if (memcmp(lb_id.mode,LB_FLATID,4)==0)
    lb_mode = LB_FLATMD;
  else if (memcmp(lb_id.mode,LB_MLZOID,4)==0)
    lb_mode = LB_MLZOMD;
  else
    lb_error("Missing, corrupted or incompatible script overlay");

  /* getting mode parameters subpass */
  lua_pushlstring(L,lb_id.nlen,10);
  lb_nlen = lua_tointeger(L,-1);
  lua_pop(L,1);
  if (lb_nlen == 0)
    lb_error("Missing, corrupted or incompatible script overlay");
  if (lb_mode == LB_MLZOMD) {
    lua_pushlstring(L,lb_id.dlen,10);
    lb_dlen = lua_tointeger(L,-1);
    lua_pop(L,1);
  }

  /* data loading subpass */
  if (fseek(lb_file,-(sizeof(lb_id)+lb_nlen),SEEK_END)!=0)
    lb_cannot("seek",argv[0]);
  char *lb_data = (char*)alloca(lb_nlen);
  if (lb_data == NULL) lb_error("Not enough memory");
  if (fread(lb_data,lb_nlen,1,lb_file)!=1)
    lb_cannot("read",argv[0]);
  fclose(lb_file);

  /* decompress pass */
  if (lb_mode == LB_MLZOMD) {
    int r;
    lzo_uint new_len;
    lzo_bytep out = (lzo_bytep)alloca(lb_dlen);
    if (out == NULL) lb_error("Not enough memory");
    r = lzo1x_decompress(lb_data,lb_nlen,out,&new_len,NULL);
    if (r == LZO_E_OK || lb_dlen == new_len)
      lua_pushlstring(L,(const char*)out,lb_dlen);
    else
      lb_error("decompress failed");
  } else lua_pushlstring(L,lb_data,lb_nlen);

  lua_newtable(L);
  {
    int i = 0;
    for (; i <= argc; i++) {
      lua_pushstring(L,argv[i]);
      lua_rawseti(L,-2,i);
    }
  }
  lua_setglobal(L,"arg");

  /* runtime pass */
  if (luaL_loadbuffer(L,lua_tostring(L,-1),lua_objlen(L,-1),argv[0]))
    lb_error(lua_tostring(L,-1));
  int i = 1;
  for (; argv[i]; i++)
    lua_pushstring(L,argv[i]);
  if (lua_pcall(L,i-1,0,0))
    lb_error(lua_tostring(L,-1));
  lua_pop(L,1);

  LBCONF_USERFUNC_DONE(L);
  lua_close(L);
}
