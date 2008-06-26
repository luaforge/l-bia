#!/usr/bin/env lua
--
--  "$Id: l-bia.lua,v 1.2 2008-06-26 23:38:16 br_lemes Exp $"
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

-- ignore any error
pcall(require,"lbaux")

-- values
local LB_NAME      = "L-Bia"
local LB_VERSION   = "0.1.10"
local LB_LONGNAME  = "Lua Buit-In program"
local LB_COPYRIGHT = "Copyright (c) 2007,2008 Breno Ramalho Lemes"

local modes   = {"-cf","-cl","-sf","-sl","-df","-dl","-rf","-rl"}
local options = {"-i","-o","-v","-h","--best"}

local mdesc = {
  ["-cf"]="Clean and Flat mode (default soft clean)",
  ["-cl"]="Clean and LZO mode (clean compress)",
  ["-sf"]="Strip and Flat mode (lstrip hard clean)",
  ["-sl"]="Strip and LZO mode (lstrip and compress)",
  ["-df"]="Dumped and Flat mode (precompile)",
  ["-dl"]="Dumped and LZO mode (precompile and compress)",
  ["-rf"]="Raw copy and Flat mode (keep blanks and comments)",
  ["-rl"]="Raw copy and LZO mode (keep and compress)",
}
local odesc = {
  ["-i"]="<file> Read input from <file>",
  ["-o"]="<file> Write output to <file>",
  ["-v"]="Show version number and exit (synonym for --version)",
  ["-h"]="Show this help message and exit (synonym for --help)",
  ["--best"]="Select the best mode available (-sl or -cf)",
}

-- functions
function lb_error(msg)
  if msg then print("ERROR: "..msg..".") end
  error()
end

function lb_cannot(what,...)
  if arg[1] then return unpack(arg) end
  lb_error("Cannot "..what.." "..arg[2])
end

local LUA_DIRSEP = '/'
local LUA_PATH_MARK = '?'

local function findfile(name, pname)
  name = string.gsub(name, "%.", LUA_DIRSEP)
  local path = package[pname]
  assert(type(path) == "string", string.format("package.%s must be a string", pname))
  for c in string.gmatch(path, "[^;]+") do
    c = string.gsub(c, "%"..LUA_PATH_MARK, name)
    local f = io.open(c)
    if f then
      f:close()
      return c
    end
  end
  return nil -- not found
end

function center(str,size)
  str = string.rep(" ",(size-#str)/2)..str..string.rep(" ",(size-#str)/2)
  if #str < size then str = " "..str end
  if #str > size then str = string.sub(str,1,-2) end
  return str
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

function usage(progname)
  print(LB_LONGNAME.." ("..LB_NAME..") "..LB_VERSION)
  print("Usage: "..progname.." [mode|option] [-i <file>] [-o <file>] <file>")
  print()
  print("Modes:")
  for i = 1,#modes do
    local k = modes[i]
    local v = mdesc[k]
    print("  "..k..string.rep(" ",8-#k)..v)
  end
  print("Options:")
  for i = 1,#options do
    local k = options[i]
    local v = odesc[k]
    print("  "..k..string.rep(" ",8-#k)..v)
  end
  print()
  print("L-Bia comes with ABSOLUTELY NO WARRANTY; This is free software, and you are")
  print("welcome to redistribute it under certain conditions; see COPYING for details.")
end

local LB_TFHELP = "Type '"..filename_name(arg[0]).." --help' for help"

function catch_modes()
  for i = 1,#modes do
    if arg[1] == modes[i] then
      return modes[i]
    end
  end
  lb_error("Invalid mode or option '"..arg[1].."'.\n"..LB_TFHELP)
end

function getopts()
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
        elseif arg[1] == "--best" then
          if not mode then
            if lbaux then
              if lbaux.lstrip then mode = "-sl" end
            else mode = "-cf" end
          else
            lb_error("Invalid mode or option '"..arg[1].."'.\n"..LB_TFHELP)
          end
        elseif arg[1] == "-v" or arg[1] == "--version" then
          print(LB_NAME.." "..LB_VERSION.." "..LB_COPYRIGHT)
          return nil
        elseif arg[1] == "-h" or arg[1] == "--help" then
          usage(filename_name(arg[0]))
          return nil
        elseif not mode then 
          mode = catch_modes()
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
  mode = mode or "-cf"
  if string.sub(mode,3,3) == "l" and (not lbaux) then
    lb_error("mini LZO real-time data compression library not available")
  end
  if string.sub(mode,2,2) == "s" and (not lbaux) then
    lb_error("lstrip not available")
  end
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

-- BEGIN clean up code
-- Based on code by Waldemar Celes - TeCGraf/PUC-Rio Jul 1999

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

function mask(s)
  for i = 1,#MASK  do
    s = string.gsub(s,MASK[i][2],MASK[i][1])
  end
  return s
end

function unmask(s)
  for i = 1,#MASK  do
    s = string.gsub(s,MASK[i][1],MASK[i][2])
  end
  return s
end

function check(s)
  local code = "return function (...)\n"..s.."\nend"
  local f,e = loadstring(code)
  if not f then lb_error(e) end
  local a,b = pcall(f)
  if not a then lb_error(b) end
end

function clean(s)
--  do checking im main() function
--  check(s)

  local S = "" -- saved string

  s = mask(s)

  -- remove blanks and comments
  while 1 do
    local b,e,d = string.find(s,ANY)
    if b then
      S = S..string.gsub(string.sub(s,1,b-1),"[ \t]+"," ") -- eliminate unecessary spaces
      s = string.sub(s,b+1)
      if d==STR1 or d==STR2 then
        e = string.find(s,d)
        S = S..d..string.sub(s,1,e)
        s = string.sub(s,e+1)
      elseif d==REM1 then
        e = string.find(s,STR4)
        s = string.sub(s,e+1)
      elseif d==STR3 then
        e = string.find(s,STR4)
        S = S..d..string.sub(s,1,e)
        s = string.sub(s,e+1)
      elseif d==REM2 then
        s = string.gsub(s,"[^\n]*(\n?)","%1",1)
      end
    else
      S = S..s
      break
    end
  end
  -- eliminate unecessary spaces
  S = string.gsub(S,"[ \t]*\n[ \t]*","\n")
  S = string.gsub(S,"\n+","\n")
  S = unmask(S)
  return S
end
-- END clean-up code

-- return a table of requires (based on clean-up code)
function reqtab(s)
  local reqpat = {
    "require%s-%(?%s-",
    "pcall%s-%(%s-require%s-,%s-",
  }
  local result = { }
  s = mask(s)
  while 1 do
    b,e,d = string.find(s,ANY)
    if b then
      local p = string.sub(s,1,b-1)
      s = string.sub(s,b+1)
      if d == STR1 or d == STR2 then
        e = string.find(s,d)
        for i = 1, #reqpat do
          if string.find(p,reqpat[i]) then
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
        for i = 1, #reqpat do
          if string.find(p,reqpat[i]) then
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

-- return string,nlen,dlen,mode
local mfunc = {
  ["-cf"]=function (s)
            local str = clean(s)
            return str,#str,#str,"Flat"
          end,
  ["-cl"]=function (s)
            s = clean(s)
            local mode = "mLZO"
            local str = lbaux.compress(s)
            -- don't compress incompressible files
            if #str > #s then
              str = s
              mode = "Flat"
            end
            return str,#str,#s,mode
          end,
  ["-sf"]=function (s)
            local str = lbaux.lstrip(s)
            return str,#str,#str,"Flat"
          end,
  ["-sl"]=function (s)
            s = lbaux.lstrip(s)
            local mode = "mLZO"
            local str = lbaux.compress(s)
            -- don't compress incompressible files
            if #str > #s then
              str = s
              mode = "Flat"
            end
            return str,#str,#s,mode
          end,
  ["-df"]=function (s)
            local str = string.dump(loadstring(s))
            return str,#str,#str,"Flat"
          end,
  ["-dl"]=function (s)
            s = string.dump(loadstring(s))
            local mode = "mLZO"
            local str = lbaux.compress(s)
            -- don't compress incompressible files
            if #str > #s then
              str = s
              mode = "Flat"
            end
            return str,#str,#s,mode
          end,
  ["-rf"]=function (s) return s,#s,#s,"Flat" end,
  ["-rl"]=function (s)
            local mode = "mLZO"
            local str = lbaux.compress(s)
            -- don't compress incompressible files
            if #str > #s then
              str = s
              mode = "Flat"
            end
            return str,#str,#s,mode
          end,
}

function showinfo(len1,len2,type,name)
  if len2 then
    print(string.format("  %10d --> %10d  %s  %s",len1,len2,
                        center(type,11),name))
  else
    print(string.format("  %s  %s  %s",
                        center(tostring(len1),25),
                        center(type,11),name))
  end
end

function main(input,script,output,mode)
  if not input then return end

  -- read input
  local i_handle = lb_cannot("open",io.open(input,"rb"))
  local i_string = lb_cannot("read",i_handle:read("*a"))
  i_handle:close()
  i_len1 = #i_string

  -- get sign table
  local i_sign = {
    sign = string.sub(i_string,-33,-28),
    mode = string.sub(i_string,-27,-24),
    nlen = string.sub(i_string,-22,-13),
    dlen = string.sub(i_string,-11,-2),
    slen = 34
  }

  if i_sign.sign == "L-Bia#" then
    if (string.sub(mode,2,2) == "x") and (string.sub(mode,3,3) == "s") then
      i_string = string.sub(i_string,-(i_sign.nlen+i_sign.slen)) -- source
    else
      i_string = string.sub(i_string,1,-(i_sign.nlen+i_sign.slen)-1) -- binary
    end
  elseif string.sub(mode,2,2) == "x" then
    lb_error("Missing, corrupted or incompatible script overlay")    
  end

  local i_len2 = #i_string
  local s_len1, s_len2, o_len
  local info_table = { }

  -- read script
  local s_handle = lb_cannot("open",io.open(script,"rb"))
  local s_string = lb_cannot("read",s_handle:read("*a"))
  s_handle:close()
  s_len1 = #s_string

  -- strip #! from first line
  if string.sub(s_string,1,2) == "#!" then
    local _,i = string.find(s_string,"^#!.-\n")
    s_string = string.sub(s_string,i,-1)
  end

  check(s_string)

  local s_reqtab = reqtab(s_string)
  local s_reqstr = ""
  for i,name in ipairs(s_reqtab) do
    local temp = { }
    temp.name = findfile(name,"path")
    if temp.name then 
      temp.type = "Lua module"
    else
      temp.name = findfile(name,"cpath")
      if temp.name then
        temp.type = "C module"
      else
        lb_error("Module not found")
      end
    end
    local s_reqhan = lb_cannot("open",io.open(temp.name,"rb"))
    local s_reqdat = s_reqhan:read("*a")
    s_reqhan:close()
    temp.len1 = #s_reqdat
    if temp.type == "Lua module" then
      s_reqdat = lbaux.quote(mfunc["-"..string.sub(mode,2,2).."f"](s_reqdat))
    else -- if C module
      s_reqdat = lbaux.quote(s_reqdat)
    end
    s_reqstr = s_reqstr..string.format("LB_REQ[%d]={module=%q,name=%q,type=%q,data=%s}\n",
                                        i,name,filename_name(temp.name),temp.type,s_reqdat)
    temp.len2 = #s_reqdat
    table.insert(info_table,temp)
  end
  if #s_reqtab > 0 then
    s_string = string.format("LB_REQ={}\n%s\n"..[[
LB_REQ.temp = os.getenv("TEMP") or os.getenv("TMP") or "/tmp"
LB_REQ.temp = LB_REQ.temp.."/"
package.cpath = LB_REQ.temp..%q..";"..package.cpath
for i,v in ipairs(LB_REQ) do
  if v.type == "Lua module" then
    loadstring(v.data)(v.module)
  else -- if C module
      local fhandle = io.open(LB_REQ.temp..v.name,"wb+")
      fhandle:write(v.data)
      fhandle:close()
      require(v.module)
      os.remove(LB_REQ.temp..v.name)
  end
end
%s]],s_reqstr,string.match(package.cpath,"%?%....?"),s_string)
  end

  -- process mode
  local s_string,s_nlen,s_dlen,s_mode = mfunc[mode](s_string)

  -- write output
  local o_handle = lb_cannot("open",io.open(output,"wb+"))
  local o_string = string.format("%s%s<L-Bia#%s:%010d:%010d>",
    i_string,s_string,s_mode,s_nlen,s_dlen)
  lb_cannot("write",o_handle:write(o_string))
  if (lbaux and lbaux.chmod) then
    lb_cannot("chmod",lbaux.chmod(o_handle,755))
  end
  o_handle:close()

  s_len2 = s_nlen
  o_len  = #o_string

--  if not o_len then return end

  print(LB_NAME.." "..LB_VERSION.." - "..mdesc[mode])
  print(LB_COPYRIGHT)
  print()
  print("          File size           File type   File name")
  print("  -------------------------  -----------  --------------------")
  showinfo(i_len1,i_len2,"Input",filename_name(input))
  for i,info in ipairs(info_table) do
    showinfo(info.len1,info.len2,info.type,filename_name(info.name))
  end
  if script then showinfo(s_len1,s_len2,"Script",filename_name(script)) end
  print("  -------------------------  -----------  --------------------")
  showinfo(o_len,nil,"Output",filename_name(output))
end

if io and string and table then
  if #arg == 0 then
    usage(filename_name(arg[0]))
  else
    main(getopts())
  end
else
  lb_error("Packages io, string or table not found")
end
