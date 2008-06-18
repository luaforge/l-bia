/*
 *  "$Id: lbaux.c,v 1.1 2008-06-18 15:53:17 br_lemes Exp $"
 *  Auxiliary library for the Lua Built-In program (L-Bia)
 *  A self-running Lua interpreter. Use it to get your Lua program, your
 *  C/C++ user code and a Lua interpreter into a single, stand-alone program.
 *  Copyright (c) 2008 Breno Ramalho Lemes
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

#ifndef LBAUX_C
#define LBAUX_C

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lbconf.h"

static int lbaux_quote(lua_State* L) {
  size_t n; int i; luaL_Buffer b;
  const char *s = luaL_checklstring(L,1,&n);
  luaL_buffinit(L,&b);
  luaL_addstring(&b,"\"");
  for (i = 0; i < n; i++) {
    int c=s[i];
    switch (c) {
      case    '"': luaL_addstring(&b,"\\\""); break;
      case   '\\': luaL_addstring(&b,"\\\\"); break;
      case   '\n': luaL_addstring(&b,"\\n"); break;
      case   '\r': luaL_addstring(&b,"\\r"); break;
      default: luaL_addchar(&b,c);
    }
  }
  luaL_addstring(&b,"\"");
  luaL_pushresult(&b);
  return 1;
}

#ifdef LBCONF_LSTRIP

/* Based on code by Luiz Henrique de Figueiredo */

#include "ldo.h"
#include "lfunc.h"
#include "llex.h"
#include "lobject.h"
#include "lparser.h"
#include "lstring.h"
#include "ltable.h"
#include "lzio.h"

typedef struct LoadS {
  const char *s;
  size_t size;
} LoadS;

static const char *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  (void)L;
  if (ls->size == 0) return NULL;
  *size = ls->size;
  ls->size = 0;
  return ls->s;
}

static void lbaux_tsquote(lua_State* L,const TString *ts) {
  const char *s = getstr(ts);
  size_t n = ts->tsv.len;
  lua_pushcfunction(L,lbaux_quote);
  lua_pushlstring(L,s,n);
  lua_call(L,1,1);
}

#define lbaux_pair(a,b) (1024*(a)+(b))

static int lbaux_clash[]= {
  lbaux_pair('-', '-'),
  lbaux_pair('.', '.'),
  lbaux_pair('.', TK_CONCAT),
  lbaux_pair('.', TK_DOTS),
  lbaux_pair('.', TK_NUMBER),
  lbaux_pair('<', '='),
  lbaux_pair('=', '='),
  lbaux_pair('>', '='),
  lbaux_pair('[', '='),
  lbaux_pair('[', '['),
  lbaux_pair('~', '='),
  lbaux_pair(TK_CONCAT, '.'),
  lbaux_pair(TK_CONCAT, TK_CONCAT),
  lbaux_pair(TK_CONCAT, TK_DOTS),
  lbaux_pair(TK_CONCAT, TK_NUMBER),
  lbaux_pair(TK_NAME, TK_NAME),
  lbaux_pair(TK_NAME, TK_NUMBER),
  lbaux_pair(TK_NUMBER, '.'),
  lbaux_pair(TK_NUMBER, TK_CONCAT),
  lbaux_pair(TK_NUMBER, TK_DOTS),
  lbaux_pair(TK_NUMBER, TK_NAME),
  lbaux_pair(TK_NUMBER, TK_NUMBER),
  0
};

static int lbaux_space(int a, int b) {
  int i,c;
  if (a >= FIRST_RESERVED && a <= TK_WHILE) a = TK_NAME;
  if (b >= FIRST_RESERVED && b <= TK_WHILE) b = TK_NAME;
  c = lbaux_pair(a,b);
  for (i = 0; lbaux_clash[i] != 0; i++)
    if (c == lbaux_clash[i]) return 1;
  return 0;
}

static int lbaux_lstrip(lua_State* L) {
  const char* ss = luaL_checkstring(L,1);
  int sp = lua_isboolean(L,2) ? lua_toboolean(L,2) : 0; /* preserve line break? */

  LoadS ls;
  ls.s = ss;
  ls.size = lua_objlen(L,1);

  int ln = 1; int lt = 0;
  ZIO Z;
  LexState X;
  FuncState F;
  Mbuffer buff;
  luaZ_init(L,&Z,getS,&ls);
  luaZ_initbuffer(L,&buff);
  X.buff = &buff;
  luaX_setinput(L,&X,&Z,luaS_new(L,"?"));
  X.fs = &F;
  X.fs->h = luaH_new(L,0,0);
  sethvalue2s(L,L->top,X.fs->h);
  incr_top(L);
  luaL_Buffer b;
  luaL_buffinit(L,&b);
  for (;;) {
    int t;
    luaX_next(&X);
    t = X.t.token;
    if (sp) { /* preserve line break? */
      if (X.linenumber != ln) {
        luaL_addstring(&b,"\n");
        ln = X.linenumber;
        lt = 0;
      }
    }
    if (lbaux_space(lt,t)) luaL_addstring(&b," ");
    switch (t) {
      case TK_EOS:
        luaL_pushresult(&b);
        return 1;
      case TK_STRING:
        lbaux_tsquote(L,X.t.seminfo.ts);
        luaL_addvalue(&b);
        break;
      case TK_NAME:
        luaL_addstring(&b,getstr(X.t.seminfo.ts));
        break;
      case TK_NUMBER:
        lua_pushfstring(L,"%s",X.buff->buffer);
        luaL_addvalue(&b);
        break;
      default:
        if (t < FIRST_RESERVED)
          luaL_addchar(&b,t);
        else
          luaL_addstring(&b,luaX_tokens[t-FIRST_RESERVED]);
        break;
    }
    lt = t;
  }
  luaL_pushresult(&b);
  return 1;
}

#endif

#ifdef LBCONF_LZO

#include "minilzo.h"

#define HEAP_ALLOC(var,size) \
  lzo_align_t __LZO_MMODEL var [((size)+(sizeof(lzo_align_t)-1))/sizeof(lzo_align_t)]
static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);

static int lbaux_compress(lua_State *L) {
  int r;
  size_t in_len;
  lzo_uint out_len;
  lzo_bytep in = (lzo_bytep)luaL_checklstring(L,1,&in_len);
  out_len = (in_len+in_len/16+64+3);
  lzo_bytep out = (lzo_bytep)alloca(out_len);
  if (out == NULL) { lua_pushnil(L); return 1; }
  r = lzo1x_1_compress(in,in_len,out,&out_len,wrkmem);
  if (r == LZO_E_OK)
    lua_pushlstring(L,(const char*)out,out_len);
  else
    lua_pushnil(L);
  return 1;
}

static int lbaux_decompress(lua_State *L) {
  int r;
  size_t out_len;
  lzo_uint new_len;
  lzo_bytep out = (lzo_bytep)luaL_checklstring(L,1,&out_len);
  lzo_uint in_len = (lzo_uint)luaL_checknumber(L,2);
  lzo_bytep in = (lzo_bytep)alloca(in_len);
  if (out == NULL) {
    lua_pushnil(L);
    return 1;
  }
  r = lzo1x_decompress(out,out_len,in,&new_len,NULL);
  if (r == LZO_E_OK && in_len == new_len)
    lua_pushlstring(L,(const char*)in,in_len);
  else
    lua_pushnil(L);
  return 1;
}

#endif

#ifdef LBCONF_CHMOD

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

static int lbaux_chmod(lua_State *L) {
  const char *filename = luaL_checkstring(L,1);
  mode_t mode = luaL_checknumber(L,2);
  int r = chmod(filename,mode);
  if (r == -1) {
    int en = errno;
    lua_pushnil(L);
    if (filename)
      lua_pushfstring(L,"%s: %s",filename,strerror(en));
    else
      lua_pushfstring(L,"%s",strerror(en));
    lua_pushinteger(L,en);
    return 3;
  }
  return 1;
}

#endif

static const luaL_Reg lbauxlib[] = {
  {"quote",lbaux_quote},
#ifdef LBCONF_LSTRIP
  {"lstrip",lbaux_lstrip},
#endif
#ifdef LBCONF_LZO
  {"compress",lbaux_compress},
  {"decompress",lbaux_decompress},
#endif
#ifdef LBCONF_CHMOD
  {"chmod",lbaux_chmod},
#endif
  {NULL,NULL}
};

int luaopen_lbaux(lua_State *L) {
  luaL_register(L,"lbaux",lbauxlib);
  return 1;
}

#endif

/*
 *  End of "$Id: lbaux.c,v 1.1 2008-06-18 15:53:17 br_lemes Exp $".
 */
