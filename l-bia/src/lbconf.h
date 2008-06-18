/*
 *  "$Id: lbconf.h,v 1.1 2008-06-18 15:53:17 br_lemes Exp $"
 *  Config file for the Lua Built-In program (L-Bia)
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

#ifndef LBCONF_H
#define LBCONF_H

#define LBCONF_LIB_BASE
#define LBCONF_LIB_LOAD
#define LBCONF_LIB_TABLE
#define LBCONF_LIB_IO
#define LBCONF_LIB_OS
#define LBCONF_LIB_STRING
#define LBCONF_LIB_MATH
#define LBCONF_LIB_DB

#define LBCONF_LSTRIP
#define LBCONF_LZO
#ifndef _WIN32
// WIN32 doesn't need it
#define LBCONF_CHMOD
#endif

//#include "usercode.c"
//#define LBCONF_USERFUNC_INIT(L) userfunc_init(L)
//#define LBCONF_USERFUNC_DONE(L) userfunc_done(L)

// If you want lbaux built-in uncomment the following
//#include "lbaux.c"
//#define LBCONF_USERFUNC_INIT(L) luaopen_lbaux(L); lua_pop(L,1); \
//                                userfunc_init(L)

#ifndef LBCONF_USERFUNC_INIT
#define LBCONF_USERFUNC_INIT(L)
#endif
#ifndef LBCONF_USERFUNC_DONE
#define LBCONF_USERFUNC_DONE(L)
#endif

#endif

/*
 *  End of "$Id: lbconf.h,v 1.1 2008-06-18 15:53:17 br_lemes Exp $".
 */
