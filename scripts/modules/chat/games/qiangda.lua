module(..., package.seeall)

local basepath = require ("helper").basepath
local gh = require("chat.GameHandler")
local sp = require("common.Util").scriptPath
local Util = require("common.Util")
local GD = require("chat.GameDefine")

qdSession = qdSession or {}
qdDict = qdDict or {}

function getQA()
    InitDict()
    --get next question
    local l = Util.getRand(qdDict)
    local q = l:split("|")
    Util.PrintTable(q)
    qdSession.q = q[1]
    qdSession.a = q[2]
end
function start()
    print("qiangda start")
    getQA()    
    return "问题:"..qdSession.q
end

function OnGame(msg)
    --search stop cmd
    print("qiangda continue")
    for n,cmd in  ipairs(GD.GAMES["qiangda"].stopCmd) do
        if msg == cmd then
            return stop()
        end
    end
    InitDict()
    local ret = ""
    --test str and answer
    local preA = qdSession.a
    getQA()
    if not (preA:find(msg) == nil ) then
        ret = "答对了,下一题:"..qdSession.q
    else
        print("wrong,correct:"..preA)
         
        ret = ret .. "下一题:"..qdSession.q
        ret = ret .. "上題答错了,正确答案:" 
        ret = ret .. preA
        --ret = string.gsub(ret,"\n","")
    end
    return ret
end

function stop()
    print("qiangda stop")
    qdSession = {}
    qdDict = {}
    gh.gameSessions = {}
    gh.currentGame = {}
end

function InitDict()
    if table.isEmpty(qdDict) then
        local fileName = basepath() .. "scripts/modules/chat/games/qiangdaDict.txt"
        qdDict = Util.line2table(fileName)
    end
end
