module(..., package.seeall)

gameSessions = gameSessions or {}
local Msg = require("common.Msg")
local GD = require("chat.GameDefine")
currentGame = currentGame or {}
local sp = require("common.Util").scriptPath
local PrintTable = require("common.Util").PrintTable

function gameDispatch(con,str)
    --查看是否有遊戲正在進行中
    if not table.isEmpty(currentGame) then
        print("game continue:"..sp()..debug.getinfo(1).currentline)
        --Msg.WorldBroadCast(currentGame.OnGame(str))
        return currentGame.OnGame(str)
    end

    --沒有遊戲正在進行中
    --查看是否是開始遊戲或結束遊戲命令
    --如果是開始遊戲執行相應的gameDispatch
    --如果是結束遊戲同樣執行相應gameDispatch
    local gameMsg = checkGameOrder(con,str)
    if gameMsg == nil then
        return nil
    end
    --print("in game progress msg:"..sp()..debug.getinfo(1).currentline )
    return gameMsg
    --将str传给正在进行中的游戏的handler
end

-- 檢查是否是開始或結束遊戲的命令
function checkGameOrder(con,msg)
    local retMsg = ""
    local findGame = sessionFind()
    if findGame == "" then
        for game,v in pairs(GD.GAMES) do -- 没有游戏在进行中，查找start命令
            for n,cmd in ipairs(v.startCmd) do
                if msg == cmd then
                    print("get game start cmd:"..msg..debug.getinfo(1).currentline)
                    if gameSessions[game] == nil then
                        currentGame =  assert(require("chat.games." .. game),"module: chat.games." .. game .. " load failed check module chat.games." .. game)
                        --Msg.WorldBroadCast( currentGame.start())
                        return currentGame.start()
                    end
                end
            end
        end
    end
    return nil
end

function sessionFind()
    if not currentGame == "" or currentGame == {} then
        return currentGame
    end
    --print("in sessionFind:"..sp()..debug.getinfo(1).currentline)
    --PrintTable(gameSessions)
    
    for k,v  in pairs(gameSessions) do
        --print('current game:',k,sp()..':'..debug.getinfo(1).currentline)
        return k
    end
    return ""
end
        

function sessionAdd(game)
    gameSessions[game] = {startTime = os.time()}
    currentGame =  assert(require("chat.games." .. game),"module: chat.games." .. game .. " load failed check module chat.games." .. game)
end

