/*
 *  "$Id: l-bia.c,v 1.1 2008-06-18 15:53:17 br_lemes Exp $"
 *  Lua Built-In program (L-Bia)
 *  A self-running Lua interpreter. Use it to get your Lua program, your
 *  C/C++ user code and a Lua interpreter into a single, stand-alone program.
 *  Copyright (c) 2007,2008 Breno Ramalho Lemes
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Breno Ramalho Lemes
 *  <br_lemes@yahoo.com.br>
 *  http://l-bia.luaforge.net/
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef __cplusplus
}
#endif

#include "lbconf.h"

#ifdef LBCONF_LZO
#include "minilzo.h"
#endif

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

#ifdef LBCONF_LZO
  if (lzo_init() != LZO_E_OK)
    lb_error("Internal LZO error");
#endif

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
#ifdef LBCONF_LZO
    lb_mode = LB_MLZOMD;
#else
    lb_error("mini LZO real-time data compression library not available");
#endif
  else
    lb_error("Missing, corrupted or incompatible script overlay");

  /* getting mode parameters subpass */
  lua_pushlstring(L,lb_id.nlen,10);
  lb_nlen = lua_tointeger(L,-1);
  lua_pop(L,1);
  if (lb_nlen == 0)
    lb_error("Missing, corrupted or incompatible script overlay");
#ifdef LBCONF_LZO
  if (lb_mode == LB_MLZOMD) {
    lua_pushlstring(L,lb_id.dlen,10);
    lb_dlen = lua_tointeger(L,-1);
    lua_pop(L,1);
  }
#endif

  /* data loading subpass */
  if (fseek(lb_file,-(sizeof(lb_id)+lb_nlen),SEEK_END)!=0)
    lb_cannot("seek",argv[0]);
  char *lb_data = (char*)alloca(lb_nlen);
  if (lb_data == NULL) lb_error("Not enough memory");
  if (fread(lb_data,lb_nlen,1,lb_file)!=1)
    lb_cannot("read",argv[0]);
  fclose(lb_file);

#ifdef LBCONF_LZO
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
#else
  lua_pushlstring(L,lb_data,lb_nlen);
#endif

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

/*
 *  End of "$Id: l-bia.c,v 1.1 2008-06-18 15:53:17 br_lemes Exp $".
 */
