module(..., package.seeall)

local TD = require("chat.ToolDefine")
local sp = require("common.Util").scriptPath
local PrintTable = require("common.Util").PrintTable
toolSessions = toolSessions or {}
currentTool = currentTool or {}

function toolDispatch(str)
    if not table.isEmpty(currentTool)  then
        return currentTool.OnTool(str)
    end

    --没有工具正在进行中，查看是否是其它工具命令
    local toolMsg = checkToolOrder(str)
    if toolMsg == nil then
        return nil
    end
    return toolMsg
end

function checkToolOrder(msg)
    local retMsg = ""
    --print('find game:'..findGame..':'..sp()..debug.getinfo(1).currentline)
    for tool,v in pairs(TD.TOOLS) do -- 没有游戏在进行中，查找start命令
        for n,cmd in ipairs(v.startCmd) do
            if msg == cmd then
                print("get cmd:"..msg..debug.getinfo(1).currentline)
                currentTool =  assert(require("chat.tools." .. tool),"module: chat.tools." .. tool .. " load failed check module chat.tools." .. tool)
                return currentTool.start()
            end
        end
    end
    return nil
end

