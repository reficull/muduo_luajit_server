module(..., package.seeall)

local getRand = require("common.Util").getRand
local basepath = require ("helper").basepath
local sp = require("common.Util").scriptPath
local PrintTable = require("common.Util").PrintTable
local TD = require("chat.ToolDefine")
local TH = require("chat.ToolHandler")
xiaohuaDict = xiaohuaDict or {}

function InitDict()
    print("xiaohua dict initializing")
    for line in io.lines(basepath().."scripts/modules/chat/tools/xiaohuaDict.txt") do
        table.insert(xiaohuaDict,line)
    end
    print("xiaohuadict initializing complete"..sp()..debug.getinfo(1).currentline)
end


function start()
    print("随机笑话")
    InitDict()
    stop()
    return getRand(xiaohuaDict)
end

function stop()
    print("笑话完结")
    TH.currentTool = {}
end
