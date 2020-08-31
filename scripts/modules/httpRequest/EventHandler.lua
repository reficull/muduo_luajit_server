module(..., package.seeall)

local Dispatcher = require("common.Dispatcher")
local PacketID = require("common.PacketID")
local Msg = require("common.Msg")
local HRDefine = require("httpRequest.HRDefine")
local Util = require("common.Util")
local PrintTable = require("common.Util").PrintTable
local cjson = require("cjson")
--[20101,["http://162.243.249.22:20081/list/1345644.html?newworldorder2015070901"]]
--[20101,["http://162.243.249.22:20081/list/1345644.html?newworldorder2015070901",1]]
--[20101,["http://162.243.249.22:20080/search.php?dbname=igame&title=psv资讯&page=1&locktitle=1&opencc=0&deviceToken=(null)"]]
--[20101,["http://162.243.249.22:20080/search.php?dbname=igame&title=psv资讯&page=2&locktitle=1&opencc=0&deviceToken=(null)"]]
function OnCGGet(nObjID,oMsg)
    --local oReturnMsg = Dispatcher.ProtoContainer[PacketID.GC_GET]
    print("get msg:")
    PrintTable(oMsg)
    local http = require("socket.http")
    local ltn12 = require("ltn12")
    local t = {}
    local resp,e,b = http.request{
        url = oMsg[1], 
        sink = ltn12.sink.table(t)
    }
    --print("url return:",resp,e,b)
    local retType = 0 
    if oMsg[2] == 1 then
        print("get gz var")
        retType = HRDefine.HR_TYPE_GZIP
    end

    local retCont = table.concat(t)
    retCont = retCont .. "end"
    return {PacketID.GC_GET,nObjID,retCont,oMsg[2]}
end
