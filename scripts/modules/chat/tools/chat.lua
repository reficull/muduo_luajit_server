module(..., package.seeall)

local basepath = require ("helper").basepath
local sp = require("common.Util").scriptPath
local PrintTable = require("common.Util").PrintTable
local TD = require("chat.ToolDefine")
local TH = require("chat.ToolHandler")
local extractDict = require("common.Util").extractDict
dialogDict = dialogDict or {}
function InitDict()
    if table.isEmpty(dialogDict) then
        print("chat dict initializing")
        for line in io.lines(basepath().."scripts/modules/chat/tools/girlDialog.txt") do
            table.insert(dialogDict,line)
        end
        print("chat dict initializing complete"..sp()..debug.getinfo(1).currentline)
    end
end

function start()
    print("auto chat start"..sp()..debug.getinfo(1).currentline)
    return "开启机器人聊天"
end

function OnTool(msg)
    print("auto chat continue"..sp()..debug.getinfo(1).currentline)    
    for n,cmd in  ipairs(TD.TOOLS["chat"].stopCmd) do
        if msg == cmd then
            return stop()
        end
    end
    InitDict()

    for k,v in pairs(dialogDict) do
        if v:sub(1,msg:len()) == msg then
            print(v:sub(1,msg:len())..","..v)
            local answer = v:sub(v:find("|")+1,-1)
            return answer
        end
    end
end

function stop()
    print("auto chat stop"..sp()..debug.getinfo(1).currentline)
    TH.currentTool = {}
    InitDict = {}
    return "机器人聊天关闭"
end
