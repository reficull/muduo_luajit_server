module(..., package.seeall)
    
local basepath = require ("helper").basepath
local gh = require("chat.GameHandler")
local sp = require("common.Util").scriptPath
local Util = require("common.Util")
local GD = require("chat.GameDefine")

cySession = cySession or {} --当前成语
cyDict = cyDict or {} --成语词典数字版

function start()
    print("chengyu start!.."..sp()..debug.getinfo(1).currentline.."\n")
    gh.sessionAdd("chengyu")
    --随机找个成语加入session，并返回
    cySession["current"] = getRandCY()
    cySession["history"] = {}
    cySession["startTime"] = os.time()
    table.insert(cySession["history"],cySession["current"])
    return  "成语接龙开始，当前成语："..cySession["current"].." ,请找["..cySession["current"]:sub(-3).."]字开头成语"
   -- return cySession["current"]
    
end

function getRandCY()
    if table.isEmpty(cyDict) then InitDict() end
    local hasNext = 0
    local valid = 0
    local cy
    repeat
        repeat
            cy= string.split(cyDict[math.random(1,#cyDict)],"|")[1]
            if cy:len() > 12 then valid =1 end 
            print("rand:"..cy)
        until valid
        cyLastChar = cy:sub(-3)
        for k,v in ipairs(cyDict) do
           if v:sub(1,3) == cyLastChar then 
               print("next:"..v)
               hasNext = 1
               break 
           end 
        end
    until hasNext == 1
    return cy 
end

function InitDict()
    if table.isEmpty(cyDict) == true then
        local fileName = basepath() .. "scripts/modules/chat/games/cidict.txt"
        print("loading ecdict:"..fileName..sp()..debug.getinfo(1).currentline)
        for line in io.lines(fileName) do
            table.insert(cyDict,line)
        end
        print("load chengyu dict complete"..sp()..debug.getinfo(1).currentline)
    end
end
function OnGame(msg)

    --msg = string.trim(msg)
    print("ongame msg:"..msg..",msglen:"..msg:len()..sp()..debug.getinfo(1).currentline)
    local ret = ""
    local currentLastChar = cySession["current"]:sub(-3)
    local msgLastChar = msg:sub(-3)
    if table.isEmpty(cySession) == false then
        for n,cmd in  ipairs(GD.GAMES["chengyu"].stopCmd) do
            if msg == cmd then
                return stop()
            end
        end
        --初始化成语词典如果没有
        InitDict()

        --處理查找
        print('s?'..msg:sub(1,1))
        if msg:sub(1,1) == "s" and msg:match("s(.*)")  then
            local search = msg:match("s(.*)")  
            local astNum = 0
            for w in string.gmatch(msg,"%*+") do
                astNum = astNum + w:len()
                search = search:sub(w:len()+1)
                print("search:"..search)
            end

            print("get search:"..search..":"..sp()..debug.getinfo(1).currentline)
            astNum = astNum + 0
            local tipsArr = {}
            local num = 0
            local start = 0 
            local limit = 0 
            if astNum < 1 then 
                start = 0
                limit =3 
            else
                start = astNum *3 +1
                limit = start + 2
            end
            for k,v in ipairs(cyDict) do
                --if num > 10 then break end
                --print("compare:"..v:sub(start,limit))
                if string.sub(v,start,limit) == search then
                    --print("found:"..Util.hex2string(v))
                    table.insert(tipsArr,v)
                    --num = num+1
                end
            end
            if table.isEmpty(tipsArr) then
                if astNum == 0 then
                    return "没有["..search.."]开头的成语"..",當前成語是["..cySession["current"].."]"
                else
                    return "没有["..search.."]在第"..(astNum+1).."位的成语"..",當前成語是["..cySession["current"].."]"
                end
            end
            --Util.PrintTable(tipsArr)
            --随机获得结果集中10个
            local result10 = {}
            if #tipsArr > 10 then
                for i=1,10 do
                    table.insert(result10,Util.getRand(tipsArr)) 
                end
            else
                for i=1,10 do
                    table.insert(result10,tipsArr[i]) 
                end
            end
            return table.concat(result10,"\n")
        end
        --處理提示
        if msg == "tips" then
            --print(" current cy:"..cySession["current"]..sp()..debug.getinfo(1).currentline)
            --print(" current cy mpack:"..string.mpack(cySession["current"])..sp()..debug.getinfo(1).currentline)
            local tipsArr = {}
            local num =0 
            local skipNum =0 
            local pageNum =10 
            for k,v in ipairs(cyDict) do
                if v:sub(1,3) == currentLastChar then
                    table.insert(tipsArr,v)
                    num = num + 1
                    repeat
                        if num < skipNum then
                            break
                        end
                        if num > (skipNum + pageNum) then break end
                        --print("found:"..Util.hex2string(v:sub(1,24)..sp()..debug.getinfo(1).currentline))
                        table.insert(tipsArr,Util.hex2string(v))
                    until true 
                end
            end
            if table.isEmpty(tipsArr) then
                stop()
                ret = "没有以["..currentLastChar.."]开头的成语,本轮成语接龙结束"
                return ret
            else
                ret = table.concat(tipsArr,"\n")
                print("tips:"..ret..sp()..debug.getinfo(1).currentline)
            end
        end
        if (msg:len() < 4) then
            return "请输入4字或以上成语"..",當前成語是["..cySession["current"].."]"
        end
        if not currentLastChar == msgLastChar then
            ret = "請輸入開頭是["..currentLastChar.."]的成語"
        else
            --看是否已经 到时
            if (( os.time() - cySession["startTime"]) > GD.GAMES["chengyu"].timeLimit) then
                                
                ret = "已经超时，本次成语接龙结束"
            end
            --查找是否有這個成語
            local found = ""

            if (testCY(msg)==1) then
                cySession["startTime"] = os.time()
                --TODO:检查是否已经在本轮历史cySession["history"],已经答过不能算对
                ret = "答對了！".."请找以["..cySession["current"]:sub(-3).."]开头的成语."
                table.insert(cySession["history"],cySession["current"])
            end

            if ret == "" then
                ret = "没有这个成语:"..msg..",當前成語是["..cySession["current"].."]"
            end
        end
    else
        print("chengyu no return")
        ret = nil
    end
    return ret

end

function testCY(msg)
    print("testCy print table cySession"..sp()..debug.getinfo(1).currentline)
    Util.PrintTable(cySession)

    local flag = 0
    for k,v in ipairs(cyDict) do
        local target = v:sub(1,msg:len()) --可能不止4字，有多长取多长
        if  target == msg then
            cySession["current"] = v:sub(1,msg:len())
            flag = 1
            break
        end
    end
    return flag
end

function stop()
    print("chengyu game stop")
    cySession = {}
    cyDict = {}
    gh.gameSessions = {}
    gh.currentGame = {}
    return "chengyu game stop"
end
