/*
 *  "$Id: lbaux.c,v 1.4 2008-07-09 20:28:32 br_lemes Exp $"
 *  Auxiliary library for the Lua Built-In program (L-Bia)
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

#ifndef LBAUX_C
#define LBAUX_C

#include "lbconf.h"

/* Based on code by Luiz Henrique de Figueiredo */
/* BEGIN */

static void lbaux_quote(lua_State* L,const TString *ts) {
  int i; luaL_Buffer b;
  size_t n = ts->tsv.len; 
  const char *s = getstr(ts);
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
        lbaux_quote(L,X.t.seminfo.ts);
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
/* END */

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

static int lbaux_adler32(lua_State *L) {
  lua_pushnumber(L,
    lzo_adler32(0,(lzo_bytep)luaL_checkstring(L,1),lua_strlen(L,1)));
  return 1;
}

static int lbaux_toustr32(lua_State *L) {
  uint32_t n = luaL_checknumber(L,1);
  char buf[4];
  memcpy(buf,&n,4);
  lua_pushlstring(L,buf,4);
  return 1;
}

static int lbaux_touint32(lua_State *L) {
  size_t size = 0;
  const char *buf = luaL_checklstring(L,1,&size);
  if (size == 4) {
    uint32_t n;
    memcpy(&n,buf,4);
    lua_pushnumber(L,n);
  } else
    lua_pushnil(L);
  return 1;
}

#ifndef _WIN32

#include <sys/stat.h>
#define LUA_FILEHANDLE "FILE*"

static int lbaux_chmod(lua_State *L) {
  FILE *f = *(FILE**)luaL_checkudata(L,1,LUA_FILEHANDLE);
  mode_t mode = luaL_checknumber(L,2);
  if (fchmod(fileno(f),mode) != -1) {
    lua_pushboolean(L,1);
    return 1;
  } else {
    int en = errno;
    lua_pushnil(L);
    lua_pushfstring(L,"%s",strerror(en));
    lua_pushinteger(L,en);
    return 3;
  }
}

#endif

static const luaL_Reg lbauxlib[] = {
  {"lstrip"   ,lbaux_lstrip},
  {"compress" ,lbaux_compress},
  {"adler32"  ,lbaux_adler32},
  {"toustr32" ,lbaux_toustr32},
  {"touint32" ,lbaux_touint32},
#ifndef _WIN32
  {"chmod"    ,lbaux_chmod},
#endif
  {NULL,NULL}
};

int luaopen_lbaux(lua_State *L) {
  if (lzo_init() != LZO_E_OK)
    return luaL_error(L,"Internal LZO error");
  luaL_register(L,"lbaux",lbauxlib);
  return 1;
}

#endif
