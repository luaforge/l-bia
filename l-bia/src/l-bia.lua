#!/usr/bin/env lua
--
--  "$Id: l-bia.lua,v 1.3 2008-07-09 20:34:03 br_lemes Exp $"
--  Lua Built-In program (L-Bia)
--  A self-running Lua interpreter. It turns your Lua program with all
--  required modules and an interpreter into a single stand-alone program.
--  Copyright (c) 2007,2008 Breno Ramalho Lemes
--
--  L-Bia comes with ABSOLUTELY NO WARRANTY; This is free software, and you
--  are welcome to redistribute it under certain conditions; see COPYING
--  for details.
--
--  <br_lemes@yahoo.com.br>
--  http://l-bia.luaforge.net/
--

require("lbaux")

--require("io")
--require("table")
--require("string")
--TODO: ignore std modules

-- values
local LB_NAME      = "L-Bia"
local LB_VERSION   = "0.1.11"
local LB_LONGNAME  = "Lua Buit-In program"
local LB_COPYRIGHT = "Copyright (c) 2007,2008 Breno Ramalho Lemes"

local LB_LMODULE = 0
local LB_LUAMAIN = 1
local LB_CMODULE = 2

local lb_modes   = {"-sf","-sl","-df","-dl","-rf","-rl"}
local lb_options = {"-i","-o","-v","-h"}

local lb_mdesc = {
  ["-sf"]="Strip and Flat mode (no compression)",
  ["-sl"]="Strip and LZO compression mode (default)",
  ["-df"]="Dumped and Flat mode (precompile)",
  ["-dl"]="Dumped and LZO mode (precompile and compress)",
  ["-rf"]="Raw copy and Flat mode (keep blanks and comments)",
  ["-rl"]="Raw copy and LZO mode (keep and compress)",
}

local lb_odesc = {
  ["-i"]="<file> Read input from <file>",
  ["-o"]="<file> Write output to <file>",
  ["-v"]="Show version number and exit (synonym for --version)",
  ["-h"]="Show this help message and exit (synonym for --help)",
}

-- functions
function lb_error(msg)
  if msg then print("ERROR: "..msg..".") end
  return error()
end

function lb_cannot(what,...)
  if arg[1] then return unpack(arg) end
  return lb_error("Cannot "..what.." "..arg[2])
end

local LUA_DIRSEP = '/'
local LUA_PATH_MARK = '?'

function lb_findfile(name, pname)
  name = string.gsub(name, "%.", LUA_DIRSEP)
  local path = package[pname]
  assert(type(path) == "string", string.format("package.%s must be a string", pname))
  for c in string.gmatch(path, "[^;]+") do
    c = string.gsub(c, "%"..LUA_PATH_MARK, name)
    local f = io.open(c)
    if f then f:close() return c end
  end
  return nil -- not found
end

function filename_ext(filename)
  local b,e = string.find(filename,"%.[^\\/]*$")
  if b then
    return string.sub(filename,b,e)
  end
  return ""
end

function filename_name(filename)
  repeat
    local s,e = string.find(filename,"[\\/].*$")
    if s then filename = string.sub(filename,s+1,e) end
  until not s
  return filename
end

function filename_base(filename)
  return (string.gsub(filename,"%.[^\\/]*$",""))
end

function lb_usage(progname)
  print(LB_LONGNAME.." ("..LB_NAME..") "..LB_VERSION)
  print("Usage: "..progname.." [mode|option] [-i <file>] [-o <file>] <file>")
  print()
  print("Modes:")
  for i,v in ipairs(lb_modes) do
    local d = lb_mdesc[v]
    print("  "..v..string.rep(" ",8-#v)..d)
  end
  print("Options:")
  for i,v in ipairs(lb_options) do
    local d = lb_odesc[v]
    print("  "..v..string.rep(" ",8-#v)..d)
  end
  print()
  print("L-Bia comes with ABSOLUTELY NO WARRANTY; This is free software, and you are")
  print("welcome to redistribute it under certain conditions; see COPYING for details.")
end

local LB_TFHELP = "Type '"..filename_name(arg[0]).." --help' for help"

function lb_getopts()
  local exeext = filename_ext(arg[-1] or arg[0])
  local outext = false
  local input  = false
  local output = false
  local script = false
  local mode   = false

  while arg[1] do
    if string.sub(arg[1],1,1) == "-" then
        if arg[1] == "-i" then
          if not input and arg[2] then
            input = arg[2]
            table.remove(arg,1)
          else lb_error("Invalid arguments!\n"..LB_TFHELP) end
        elseif arg[1] == "-o" then
          if not output and arg[2] then
            output = arg[2]
            table.remove(arg,1)
          else lb_error("Invalid arguments!\n"..LB_TFHELP) end
        elseif arg[1] == "-v" or arg[1] == "--version" then
          print(LB_NAME.." "..LB_VERSION.." "..LB_COPYRIGHT)
          return nil
        elseif arg[1] == "-h" or arg[1] == "--help" then
          lb_usage(filename_name(arg[0]))
          return nil
        elseif not mode then
          for i,v in ipairs(lb_modes) do
            if arg[1] == v then
              mode = v
              break
            end
          end
          if not mode then
            lb_error("Invalid mode or option '"..arg[1].."'.\n"..LB_TFHELP)
          end
        else
          lb_error("Invalid mode or option '"..arg[1].."'.\n"..LB_TFHELP)
        end      
    elseif arg[-1] and not input then
      input = arg[1]
    elseif not script then
      script = arg[1]
    else lb_error("Too many arguments.\n"..LB_TFHELP) end
    table.remove(arg,1)
  end
  mode = mode or "-sl"
  if not script then
    lb_error("Too few arguments.\n"..LB_TFHELP)
  end
  if not output then
    output = filename_base(script)
  end
  if not input then
    if not arg[-1] then
      input = arg[0]
    else lb_error("Too few arguments.\n"..LB_TFHELP) end
  end
  outext = outext or exeext
  if filename_ext(input) == "" then input = input..exeext end
  if script and filename_ext(script) == "" then script = script..".lua" end
  if filename_ext(output) == "" then output = output..outext end
  return input,script,output,mode
end

-- Based on code by Waldemar Celes - TeCGraf/PUC-Rio Jul 1999
-- BEGIN

-- mark up comments and strings
STR1 = "\001"
STR2 = "\002"
STR3 = "\003"
STR4 = "\004"
-- long comment
REM1 = "\008"
-- short comment
REM2  = "\005"
ANY  = "([\001-\005\008])"
-- ANY ="([\001-\005])"
ESC1 = "\006"
ESC2 = "\007"

MASK = { -- the substitution order is important
  {ESC1, "\\'"},
  {ESC2, '\\"'},
  {STR1, "'"},
  {STR2, '"'}, -- "
-- long comment
  {REM1, "%-%-%[%["},
  {STR3, "%[%["},
  {STR4, "%]%]"},
-- short comment
  {REM2 , "%-%-"},
}

function lb_mask(s)
  for i,v in ipairs(MASK)  do
    s = string.gsub(s,v[2],v[1])
  end
  return s
end

function lb_check(s)
  local code = "return function (...)\n"..s.."\nend"
  local f,e = loadstring(code)
  if not f then lb_error(e) end
  local a,b = pcall(f)
  if not a then lb_error(b) end
end

-- return a table of requires (based on clean-up code)
function lb_reqtab(s)
  local reqpat = {
    "require%s-%(?%s-",
    "pcall%s-%(%s-require%s-,%s-",
  }
  local result = { }
  s = lb_mask(s)
  while 1 do
    b,e,d = string.find(s,ANY)
    if b then
      local p = string.sub(s,1,b-1)
      s = string.sub(s,b+1)
      if d == STR1 or d == STR2 then
        e = string.find(s,d)
        for i,v in ipairs(reqpat) do
          if string.find(p,v) then
            table.insert(result,string.sub(s,1,e-1))
            break
          end
        end
        s = string.sub(s,e+1)
      elseif d == REM1 then
        e = string.find(s,STR4)
        s = string.sub(s,e+1)
      elseif d == STR3 then
        e = string.find(s,STR4)
        for i,v in ipairs(reqpat) do
          if string.find(p,v) then
            table.insert(result,string.sub(s,1,e-1))
            break
          end
        end
        s = string.sub(s,e+1)
      elseif d == REM2 then
        s = string.gsub(s,"[^\n]*(\n?)","%1",1)
      else
        s = string.sub(s,b+1,-1)
      end
    else break end
  end
  return result
end

-- END

function lb_mfunc(mode,s)
  mode = string.sub(mode,2,2)
  if mode == "s" then
    return lbaux.lstrip(s)
  elseif mode == "d" then
    return string.dump(loadstring(s))
  else -- for sure it's mode == "r"
    return s
  end
end

function main(input,script,output,mode)
  if not input then return end

  -- read input
  local i_handle = lb_cannot("open",io.open(input,"rb"))
  local i_string = lb_cannot("read",i_handle:read("*a"))
  i_handle:close()

  -- open output
  local o_handle = lb_cannot("open",io.open(output,"wb"))
  local o_string = ""

  if string.sub(i_string,-16,-13) == "LB02" then
    local size = lbaux.touint32(string.sub(i_string,-4))
    o_handle:write(string.sub(i_string,1,-(size+16+1)))
  else
    o_handle:write(i_string)
  end

  -- read script
  local s_handle = lb_cannot("open",io.open(script,"rb"))
  local s_string = lb_cannot("read",s_handle:read("*a"))
  s_handle:close()

  -- strip "#!" from first line
  if string.sub(s_string,1,2) == "#!" then
    local _,i = string.find(s_string,"^#!.-\n")
    s_string = string.sub(s_string,i,-1)
  end

  lb_check(s_string)

  local s_reqtab = lb_reqtab(s_string)
  for i,name in ipairs(s_reqtab) do
    local fullname = lb_findfile(name,"path")
    if fullname then -- Lua Module
      local l_handle = lb_cannot("open",io.open(fullname,"rb"))
      local l_string = lb_cannot("read",l_handle:read("*a"))
      l_handle:close()
      lb_check(l_string)
      l_string = lb_mfunc(mode,l_string)
      name = filename_name(fullname)
      if #name > 255 then name = string.sub(name,1,255) end
      o_string = o_string..string.char(LB_LMODULE)..  -- File ID
                 string.char(#name)..name..           -- Name (size and name)
                 lbaux.toustr32(#l_string)..l_string  -- File (size and data)
    else
      fullname = lb_findfile(name,"cpath")
      if fullname then -- C Module
        local c_handle = lb_cannot("open",io.open(fullname,"rb"))
        local c_string = lb_cannot("read",c_handle:read("*a"))
        c_handle:close()
        name = filename_name(fullname)
        if #name > 255 then name = string.sub(name,1,255) end
        o_string = o_string..string.char(LB_CMODULE)..  -- File ID
                   string.char(#name)..name..           -- Name (size and name)
                   lbaux.toustr32(#c_string)..c_string  -- File (size and data)
      else lb_error("Module not found") end
    end
  end

  s_string = lb_mfunc(mode,s_string)
  local s_name = filename_name(script)
  if #s_name > 255 then s_name = string.sub(s_name,1,255) end
  o_string = o_string..string.char(LB_LUAMAIN)..  -- File ID
             string.char(#s_name)..s_name..       -- Name (size and name)
             lbaux.toustr32(#s_string)..s_string  -- File (size and data)

  local mlzosize = 0
  local flatsize = #o_string
  if string.sub(mode,3,3) == "l" then
    local n_string = lbaux.compress(o_string)
    if #n_string < #o_string then
      mlzosize = #n_string
      o_string = n_string
    end
  end

  o_handle:write(o_string,"LB02")
  if mlzosize > 0 then 
    o_handle:write(lbaux.toustr32(flatsize),
                   lbaux.toustr32(lbaux.adler32(o_string)),
                   lbaux.toustr32(mlzosize))
  else
    o_handle:write(lbaux.toustr32(0),
                   lbaux.toustr32(lbaux.adler32(o_string)),
                   lbaux.toustr32(flatsize))
  end

  -- close output
  o_handle:close()
end

if #arg == 0 then
  lb_usage(filename_name(arg[0]))
else
  main(lb_getopts())
end
