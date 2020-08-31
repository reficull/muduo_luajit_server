local helper   = require 'scripts/helper'
local basepath = helper.basepath
LUA_SCRIPT_ROOT = basepath()
function AppendPath(path)
    package.path = package.path .. string.format(";%s%s", LUA_SCRIPT_ROOT, path)
end
AppendPath("?.lua")
AppendPath("scripts/?.lua")
AppendPath("scripts/modules/?.lua")
dbg = require("debugger")
mp = require('3rd.MessagePack')
Msg = require("common.Msg")
------------------------

CppClient= CppClient or {}

function passCobj(obj)
    if CppClient == {} then
        CppClient= TCON(obj)
    end
end

function msgHandle(nObjID, nPacketID,nData)
   -- print("received:",XORCODEC(nData))
   -- print("unpack:",mp.unpack(nData))

end

function msgSend(nObjID,nPacketID,nData)
    print("conn:",nObjID)
    --local packMsg = mp.pack(nData).tostring()
    local CppClient= TCON(nObjID)
    --local msg = XORCODEC(nData)
    local msg = nData
    print("sending:",msg)
    print("ip:",CppClient)
    CppClient:sendMsg(msg)
end


