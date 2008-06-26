/*
 *  "$Id: lbconf.h,v 1.2 2008-06-26 23:38:16 br_lemes Exp $"
 *  Config file for the Lua Built-In program (L-Bia)
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

#ifndef LBCONF_H
#define LBCONF_H

#ifdef __cplusplus
extern "C" {
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef __cplusplus
}
#endif

#ifndef _WIN32
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
// WIN32 doesn't need chmod
#endif

#include "minilzo.h"

#define LBCONF_LIB_BASE
#define LBCONF_LIB_LOAD
#define LBCONF_LIB_TABLE
#define LBCONF_LIB_IO
#define LBCONF_LIB_OS
#define LBCONF_LIB_STRING
#define LBCONF_LIB_MATH
//#define LBCONF_LIB_DB

//#include "usercode.c"
//#define LBCONF_USERFUNC_INIT(L) userfunc_init(L)
//#define LBCONF_USERFUNC_DONE(L) userfunc_done(L)

// If you want lbaux built-in uncomment the following
//#include "lbaux.c"
//#define LBCONF_USERFUNC_INIT(L) luaopen_lbaux(L); lua_pop(L,1);

#ifndef LBCONF_USERFUNC_INIT
#define LBCONF_USERFUNC_INIT(L)
#endif
#ifndef LBCONF_USERFUNC_DONE
#define LBCONF_USERFUNC_DONE(L)
#endif

#endif
