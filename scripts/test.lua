#! /usr/local/bin/luajit

local helper   = require('helper')
local basepath = helper.basepath
LUA_SCRIPT_ROOT = basepath()
function AppendPath(path)
    package.path = package.path .. string.format(";%s%s", LUA_SCRIPT_ROOT, path)
end
AppendPath("?.lua")
AppendPath("scripts/?.lua")
AppendPath("scripts/modules/?.lua")
require("common.Util")
local mp = require('3rd.MessagePack')
local pt = require("common.Util").PrintTable
cjson = require("cjson")
--[[
DB = DB or require("common.DB").DB
print('before dd loaded')

d = require('dd')
d.pause()

print('after dd loaded')

local tt = {}
if table.isEmpty(tt) then
    print("tt is empty")
else
    print("tt is no emptyh")
end

local t = {
    a=1,
   b=2,
   }
local pkg = mp.pack(t)
print("pkg:",pkg)
print("unpack:")
pt(mp.unpack(pkg))
]]--
local str = '[10001,["reficul","pass","1458718922","secret"]]'
print("enc:",str)
local tb = {
    a = 123,
    b = "asdfasdf",
    c = {
       d = "asdfasdf122323",
       e = 234234234,
   },
}
print("cjson:",cjson.encode(tb))
print("mp:",mp.pack(tb))
