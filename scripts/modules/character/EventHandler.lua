module(..., package.seeall)

local Util = require("common.Util")
local Dispatcher = require("common.Dispatcher")
--local Event = require("common.Event")
local PacketID = require("common.PacketID")
local CharDB = require("character.CharDB")
local CharNames = require("character.CharNames")
local getV = require("common.Util").getValue
local PrintT = require("common.Util").PrintTable
local Msg = require("common.Msg")
local md5 = require("common.md5")
local CommonDefine = require("common.CommonDefine")

function LoginOK(human,msg)
    OnlineAccountManager[human:GetAccount()] = human
    local oHumanInfoMsg = Dispatcher.ProtoContainer[PacketID.GC_HUMAN_INFO]
    oHumanInfoMsg.rolename = human:GetName()
    oHumanInfoMsg.gender = human:GetGender()
    Msg.SendMsg(oHumanInfoMsg,human.id)
    return 
end

--account,timestamp,authkey
--[10001,["reficul","pass","1458718922","secret"]]
--[10001,["lucifer","pass","1458718922","secret"]]
function OnCGAskLogin(con,msg)
    local oReturnMsg = Dispatcher.ProtoContainer[PacketID.GC_ASK_LOGIN]
    oReturnMsg.svrName = SVRNAME
    oReturnMsg.msvrIP = MSVRIP
    oReturnMsg.msvrPort = MSVRPORT

    local account = msg.account
    local password = msg.password
--    oMsg = Msg.decodeMsg(msg,PacketID.CG_ASK_LOGIN)
    local oOldHuman = OnlineAccountManager[account]
    if oOldHuman and  oOldHuman.id == con then
       -- 重复登录，不管
        oReturnMsg.result = CommonDefine.ASK_LOGIN_OK
        --getT[1] = CommonDefine.ASK_LOGIN_OK
        --Msg.SendMsg(oReturnMsg,con)
        return 
    end
    if oOldHuman then
        -- 帐号已经登录了 把原连接挤下线
        print("login with another device,kick old user")
        oOldHuman:DoDisconnect(CommonDefine.DISCONNECT_REASON_ANOTHER_CHAR_LOGIN)
        --inform c++ side to disconnect disconnect
    end

    local human = ObjHuman:New(con, SVRNAME)
    human.m_db.ip = TCON(con):getPeerAddr()
    local ret = human:Load(account, true)
    if ret then
        print("user exists")
        oReturnMsg.result = CommonDefine.ASK_LOGIN_OK
        Msg.SendMsg(oReturnMsg,con)
        LoginOK(human,msg)
    else
        oReturnMsg.result = CommonDefine.ASK_LOGIN_CHARACTER_NONEXISTS
        Msg.SendMsg(oReturnMsg,con)
    end
    --return {PacketID.GC_ASK_LOGIN,"",rtStr,ENCODETYPE}
end

--[10003,["reficul","pass","1458718922","secret"]]
--[10003,["lucifer","pass","1458718922","secret"]]
function OnCGRegister(con,msg)
    local oReturnMsg = Dispatcher.ProtoContainer[PacketID.GC_REGISTER]
    local human = ObjHuman:New(con, SVRNAME)
    local ret = human:Load(msg.account, true)
    if not ret then
        --create new account 
        print("user not exists ")
        human:SetAccount(msg.account) -- set account name
        human:SetPassword(msg.password) -- set account name
        human.m_db.ip = TCON(con):getPeerAddr()
        human:Save()
        oReturnMsg.status = CommonDefine.REGISTER_OK

        Msg.SendMsg(oReturnMsg,con)
        LoginOK(human,msg)
    else
        oReturnMsg.status = CommonDefine.REGISTER_ACCOUNT_EXISTS
        Msg.SendMsg(oReturnMsg,con)
    end

    return 
end

--[10052,[1]]
function OnCGGetName(nObjID,msg)
    print("OnCGGetName called")
    local oReturnMsg = Dispatcher.ProtoContainer[PacketID.GC_GET_NAME]
    oReturnMsg.name = getName(msg.gender)
    Msg.SendMsg(oReturnMsg,nObjID)
end
-- career,gender,account,rolename
--[10056,[1,1,"reficul","TheGod"]]
--[10056,[1,1,"lucifer","DarkSoul"]]
function OnCGCreateRole(human,msg)
    print("OnCGCreateRole called")
    print("md5:",md5.string)
    -- compare db and save to
    --PrintT(msg)
    local oReturnMsg = Dispatcher.ProtoContainer[PacketID.GC_CREATE_ROLE]
    if CharDB.IsNameExistInDB(msg.rolename) then
        oReturnMsg.status = CommonDefine.CREATE_ROLE_NAME_EXISTS
        Msg.SendMsg(oReturnMsg,human.id)
    else
        human.m_db.name = msg.rolename
        human:AddUser()
        oReturnMsg.status = CommonDefine.CREATE_ROLE_OK
        Msg.SendMsg(oReturnMsg,human.id)
        print("new user added:",msg.rolename)
    end
    --LoginOK(human,oReturnMsg)
    
    --return {PacketID.GC_CREATE_ROLE,"",rtStr,ENCODETYPE}
end

function OnCGDisconnect(oHuman)
    print("OnCGDIsconnect called")
    oHuman:DoDisconnect("disconnect") 
    
   -- return {PacketID.GC_DISCONNECT,oHuman.id,rtStr,ENCODETYPE}
end

function OnCharRealDestroy(nTimerID, nObjID, nEvent, nParam1, nParam2, nParam3)
    print("OnCharRealDestroy", nObjID)

    local oHuman = ObjHuman:GetObj(nObjID)
    if oHuman then
        oHuman:Destroy()
    else
        _DelTimer(nTimerID, nObjID)
    end
end

function getName(oMsg)
    local gender = oMsg.gender
    if gender == 0 then
        gender = 2
    end

    local retSername = ""
    local fullname = ""

    local retname = ""
    local sernametotal = 440 -- 已知440个姓，不要再算了，如果改了再说
    local famname = ""
    local totalfamname = 0 -- 根据男或女来定
    local r = 0
    repeat
        -- 获得姓
        math.randomseed(os.time())
        r = math.random(sernametotal)
        local loop = 0;
        for i,v in pairs(CharNames.surname) do
            loop = loop + 1
            if loop == r then
                retSername = v
                break
            end
        end
        -- 获得名
        -- 判断性别
        if gender == 1 then
            namefam = CharNames.name1
            totalfamname = 360 -- 男名360个
        else
            namefam = CharNames.name2
            totalfamname = 100 -- 女名100个
        end

        r = math.random(totalfamname)
        loop = 0;
        for i,v in pairs(namefam) do
            loop = loop + 1
            if loop == r then
                retname = v
                break
            end
        end
        fullname = retSername .. retname
        nameExists = CharDB.IsNameExistInDB(fullname) 
    until nameExists ~= true
    return fullname
end
