module(..., package.seeall)
local Dispatcher = require("common.Dispatcher")
local PacketID = require("common.PacketID")
local Msg = require("common.Msg")
local ChatDefine = require("chat.ChatDefine")
local Util = require("common.Util")
local PrintTable = require("common.Util").PrintTable
local gh = require("chat.GameHandler")
local th = require("chat.ToolHandler")
local sessions = sessions or {} -- 會話狀態
local sp = require("common.Util").scriptPath

-- 返回菜单或是游戏handler的处理结果
orderFuncs = {
    showMenu = function() return ChatDefine.MAIN_MENU end,
    showGame = function() return ChatDefine.GAME_MENU end,
    tools = function(con,msg)
        return th.toolDispatch(con,msg)
    end,
    game = function(con,msg)
        --print("call game dispatcher:"..sp()..debug.getinfo(1).currentline )
        return gh.gameDispatch(con,msg)    
    end,
}
-- [10101,[1,"menu"]]
-- [10101,[1,"cy"]]
function OnCGChat(nObjID,oMsg)
    --local oReturnMsg = Dispatcher.ProtoContainer[PacketID.GC_CHAT]
    local con = getOObj(nObjID)
    local retType = 0 
    print("in OnCGChat:")
    PrintTable(oMsg)
    local msg = oMsg.msg
    if msg == nil then
        return nil
    end
    local retMsg
    local menu = findMenu(msg)    
    if menu then
        print('calling: ',menu,'()')
        retMsg = assert(orderFuncs[menu](),"menu return nil,check menu define:"..msg)--返回菜單指令結果 
        Msg.WorldBroadCast(retMsg)
        return 
    else
        --print("call game dispatcher:"..sp()..debug.getinfo(1).currentline )
        retMsg = orderFuncs.game(con,msg) --讓遊戲處理器尋找合適的結果
        if retMsg == nil then
            retMsg = orderFuncs.tools(con,msg) --讓工具處理器尋找合適結果
            if retMsg == nil then
                retMsg = msg -- 原樣返回msg去廣播
            end
        end
    end
    --local oReturnMsg = Dispatcher.ProtoContainer[PacketID.GC_BROADCAST]
    --oReturnMsg.msg = retMsg
    --oReturnMsg.pos = 1
    print("ret:",retMsg ..":".. debug.getinfo(1).currentline)
    Msg.WorldBroadCast(retMsg)
    --return retMsg
end

function findMenu(msg)
    for i,v in ipairs(ChatDefine.MENU_ALIAS) do
        if type(v) == "table" then
            for j,o in ipairs(v) do
                if msg == o then
                    --print('found:',ChatDefine.MENUS[i])
                    return ChatDefine.MENUS[i]
                end
            end
        end
    end
    return nil
end
