--[[
table manipulate
http://blog.sina.com.cn/s/blog_923fdd9b0102vap7.html
--]]
local helper   = require 'scripts/helper'
local basepath = helper.basepath
LUA_SCRIPT_ROOT = basepath()
TimeEvents = TimeEvents or {}
TimeEventsPool = TimeEventsPool or {}
AddTimerCnt = AddTimerCnt or 0

local tstamp = TSTAMP()

function _print(...)
    for i,v in ipairs(arg) do
        printResult = printResult .. tostring(v) .. "\t"
    end
    printResult = printResult .. "\n"
end

function AppendPath(path)
    package.path = package.path .. string.format(";%s%s", LUA_SCRIPT_ROOT, path)
end
RequireReal = RequireReal or myrequire
RenewAdj = RenewAdj or {}
local RenewVis
TimeEvents = TimeEvents or {}
TimeEventsPool = TimeEventsPool or {}
AppendPath("?.lua")
AppendPath("scripts/?.lua")
AppendPath("scripts/modules/?.lua")

require("config")
require("common.Obj")
require("character.ObjHuman")
dbg = require("debugger")
mp = require('3rd.MessagePack')
print("server name:" .. SVRNAME)
print("msvrip:" .. MSVRIP)
print("msvrport:" .. MSVRPORT)

--print("package path",package.path)
--if _DEBUG_ then pause("main pause") end
function RenewDfs(moduleName)
    print('renewdfs',moduleName)
  if RenewVis[moduleName] then
        return
	end
	if moduleName == "common.Dispatcher" then
		return
	end
	RenewVis[moduleName] = true
    print('on table:',moduleName,' do func:',RenewDfs)
    if type(RenewAdj[moduleName]) == 'table' then 
        table.foreach(RenewAdj[moduleName], RenewDfs)
    end
end


function Renew(moduleName)
	RenewVis = {}
	RenewDfs(moduleName)
	for k, _ in pairs(RenewVis) do
		package.loaded[k] = nil
	end
	for k, _ in pairs(RenewVis) do
		print(k, "renew ----------------------------------------")
		require(k)
	end
end
function myrequire(moduleName)
	RenewAdj[moduleName] = RenewAdj[moduleName] or {}
	local moduleNameUp = getfenv(debug.getinfo(2, "f").func)["_NAME"]
	if moduleNameUp then
		RenewAdj[moduleName][moduleNameUp] = true
    end
    print('requiring',moduleName,moduleNameUp)
		local source = debug.getinfo(2, "S").source
        print('source:',source)
	local ret = Renew(moduleName)
    print(moduleName,ret)
	if type(ret) ~= "table" then
		local source = debug.getinfo(2, "S").source
		assert(source:find("main.lua") or source:find("renew.lua"))
	end
	return ret
end

local PrintTableF = require("common.Util").PrintTableF
local PrintTable = require("common.Util").PrintTable
local CommonDefine = myrequire("common.CommonDefine")
local RequireModule = require("common.RequireModule").RegisterOneModuleProtos
function renewList()
    --can be used in production stage
    print("----- load/renew protocols ----- ")
    RequireModule("chat")
    RequireModule("httpRequest")
    RequireModule("character")
    --mostly development stage  
    print("----- load/renew funcs     ----- ")
    Renew("common.Msg")
end

local Msg = require("common.Msg")
local PacketID = require("common.PacketID")

LuaInit = function()
    print("main initializing")
    return 0
end

DB = DB or require("common.DB").DB
cjson = require("cjson")
local Dispatcher = require("common.Dispatcher")
--print("protohandler:")
--PrintTable(Dispatcher.ProtoHandler)
--PrintTable(Dispatcher.ProtoContainer)
function dp1(msg)
    print("dp1:",msg)
    tmsg = {1,1,msg}
    return tmsg
end

CppServer = CppServer or {}
function passCobj(obj)
    CppServer = C2lua(obj)
end

-- nObjID现在是conn obj
function MsgDispatch(nObjID, msgType,nData)
    local ret = {}
    local packetid,decodeData = Msg.decodeMsg(nData,msgType)
    print("main get:",decodeData)
    local nPacketID = packetid
    local OnMsg = Dispatcher.ProtoHandler[nPacketID] 
    if OnMsg then
        
        if nPacketID == PacketID.CG_ASK_LOGIN or nPacketID == PacketID.CG_CHAT or nPacketID == PacketID.CG_REGISTER then
            ret = OnMsg(nObjID, decodeData)
        else
            local oHuman = ObjHuman:GetObj(nObjID)	
            if oHuman then -- OnCGAskLogin , OnChat
                ret = OnMsg(oHuman, decodeData)
            else
                print("no objhuman")
            end
        end
    else
        print("no event handler found")
    end
    if ret == nil then
        ret = {PacketID.GC_CHAT,"","error",0}
    end
    --print("luamsg::SendMsg:",s:SendMsg())
    --s:SendMsg(nPacketID,nObjID,nData)
    return ret
end

function removeConn(nObjID)
    print("remove conn in lua")
    ObjHuman:Destroy(nObjID) 
    return 1
end


function NewTimeEventNode(objID, eventID, interval, maxTimes, p1, p2, p3, nextCallTime, timerID)
	if #TimeEventsPool < 1 then
		return {objID = objID, eventID = eventID, interval = interval, maxTimes = maxTimes, p1 = p1, p2 = p2, p3 = p3, nextCallTime = nextCallTime, timerID = timerID}
	end
	local ret = TimeEventsPool[#TimeEventsPool]
	ret.objID = objID
	ret.eventID = eventID
	ret.interval = interval
	ret.maxTimes = maxTimes
	ret.p1 = p1
	ret.p2 = p2
	ret.p3 = p3
	ret.nextCallTime = nextCallTime
	ret.timerID = timerID
	TimeEventsPool[#TimeEventsPool] = nil
	return ret
end

function DelTimeEventNode(node)
	TimeEventsPool[#TimeEventsPool + 1] = node
end

function _AddTimer(objID, eventID, interval, maxTimes, p1, p2, p3)
	AddTimerCnt = AddTimerCnt + 1
	TimeEvents[#TimeEvents + 1] = NewTimeEventNode(objID, eventID, interval, maxTimes, p1, p2, p3, tstamp:addTime(interval), AddTimerCnt)
	return AddTimerCnt
end

function _DelTimer(timerID, objID)
	if not timerID or timerID < 1 then
		return
	end
	for i = 1, #TimeEvents do
		if TimeEvents[i].timerID == timerID then
			if objID and TimeEvents[i].objID ~= objID then
				assert(nil)
			end	
			TimeEvents[i].maxTimes = 0
			return
		end
	end
	assert(nil, timerID)
end

function TimerDispatch(curTime)
    --print("timer dispatch called:",curTime)
    for i = #TimeEvents, 1, -1 do
		local node = TimeEvents[i]
		if node.maxTimes == 0 then
			DelTimeEventNode(TimeEvents[i])
			TimeEvents[i] = TimeEvents[#TimeEvents]
			TimeEvents[#TimeEvents] = nil
		elseif node.nextCallTime < curTime then
			--print("eventID", node.eventID)
			local timerHandler = Dispatcher.TimerDispatcher[node.eventID]
			timerHandler(node.timerID, node.objID, node.eventID, node.p1, node.p2, node.p3)
			node.nextCallTime = curTime + node.interval
			if node.maxTimes > 0 then
				node.maxTimes = node.maxTimes - 1
			end
		end
    end
end

Obj:RefreshOldObj()


--[[
Log = require("common.Log")

globalNum = globalNum or 100

sumNumbers = function(a,b)
    printMessage("You can still call C++ functions from Lua functions!")
    globalNum = globalNum + 100
    return a + b
end

getGlobal = function()
    return globalNum
end
]]--
