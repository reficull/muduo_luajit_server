module(..., package.seeall)
local PrintTableF = require("common.Util").PrintTableF
local PrintT = require("common.Util").PrintTable
local Dispatcher = require("common.Dispatcher")
local ProtoContainer = Dispatcher.ProtoContainer
local ProtoContainer2PacketID = Dispatcher.ProtoContainer2PacketID
local getV = require("common.Util").getValue
local mpack = require("3rd.MessagePack")
require("common.xor")
reload = 1
MSG_TYPE={
    MT_NORMAL = "0",
    MT_XOR = "1",
    MT_WEBSOCKET = "2",
}
function decodeMsg(oMsg,msgType)
    print("get mt type:",msgType)
    local ret = {}
    if msgType == MSG_TYPE['MT_XOR'] then
        oMsg = XORCODEC(oMsg)
        print("proto:xor",oMsg)
    elseif msgType == MSG_TYPE['MT_WEBSOCKET'] then
        print("proto:websocket")

    elseif msgType == MSG_TYPE['MT_NORMAL'] then
        print("proto:normal")

    end
    local ddata = assert(cjson.decode(oMsg) ,"msg not valid:"..oMsg)
    print('ddata:'..ddata)
    if ddata == nil then
        return ret
    end
    local packetID = ddata[1]
    if ProtoContainer[packetID] == nil then
        return 
    end

    local i=1
    for k,v in pairs(Dispatcher.ProtoTemplate[packetID]) do
        ret[v[1]] = ddata[2][i] 
        i = i+1
    end
    return packetID,ret
end

------------------------------ wrap into class like behavior

function SendMsgByFD(msg, fd)
 --   return g_oLuaMsgEx:SendMsgByFD(ProtoContainer2PacketID[msg], fd, msg)
end

function SendMsg(msg, objID)
    local rt = getV(msg) 
    local packetID = ProtoContainer2PacketID[msg]
--    local rtMsg = mpack.pack(msg).tostring()
    --print("mpack str:",rtMsg)
    local rtMsg = {packetID,rt}
    local rtStr = cjson.encode(rtMsg)
    if not CppServer:isWsClient(objID) then
        print("lua test is not  ws client")
        rtStr = XORCODEC(rtStr)
    end
    --return TCON(objID):sendMsg(rtStr)
    print("send to :"..objID..",str:"..rtStr)
    return CppServer:sendMsg(objID,rtStr)
end

function UserBroadCast(msg, userList)
--    return g_oLuaMsgEx:UserBroadcast(ProtoContainer2PacketID[msg], userList, msg) 
end

function ZoneBroadCast(msg, objID)
--    return g_oLuaMsgEx:ZoneBroadcast(ProtoContainer2PacketID[msg], objID, msg)
end

function SceneBroadCast(msg, sceneID)
--    return g_oLuaMsgEx:SceneBroadcast(ProtoContainer2PacketID[msg], sceneID, msg)
end

function WorldBroadCast(msg)
    local rt = getV(msg) 
    local rtMsg = {ProtoContainer2PacketID[msg],rt}
    local rtStr = cjson.encode(rtMsg)
    for k,v in pairs(OnlineObjManager) do
        print("world broadcast user:",k)
        --TCON(k):sendMsg(rtStr)
        if not CppServer:isWsClient(k) then
            print('send xorcodec str:'..rtStr)
            --CppServer:sendMsg(k,XORCODEC(rtStr))
            TCON(k):sendMsg(XORCODEC(rtStr))
            print('after send xorcodec str:'..rtStr)
        else
            print('send normal str:'..rtStr)
            TCON(k):sendMsg(rtStr)
            --CppServer:sendMsg(k,rtStr)
            print('after send normal str:'..rtStr)
        end
    end
end
