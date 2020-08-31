local CommonDefine = require("common.CommonDefine")
local Dispatcher = require("common.Dispatcher")
local PacketID = require("common.PacketID")
local Msg = require("common.Msg")
--[[
local _ObjEnterScene = _ObjEnterScene
local _GetObjType = _GetObjType
local _GetPosition = _GetPosition
local _GetDistance = _GetDistance
local _MoveTo = _MoveTo
local _ScanHuman = _ScanHuman
local _ScanHumanAndMonster = _ScanHumanAndMonster
local _ObjEnterScene = _ObjEnterScene
local _ObjLeaveScene = _ObjLeaveScene
local _ChangePosition = _ChangePosition
local _UpdateCharCacheAttr = _UpdateCharCacheAttr
]]--

OnlineHumanManager = OnlineHumanManager or {}
ObjManager = ObjManager or {} 
Obj = {}

function Obj:RefreshOldObj()	--必须在require完所有obj类后调用
	for k,v in pairs(ObjManager) do
		v:ReSetMetatable();
	end
end

function Obj:New()	
	local obj = {id=-1}
	setmetatable(obj, self)
    self.__index = self
    return obj 
end

function Obj:GetObj(nObjID)
	return ObjManager[nObjID]
end

function Obj:SetLogined()
    print "set obj login"
    --return _SetObjLogined(self.id)
end
--[[
function Obj:EnterScene(nSceneID, nX, nY)
	if self.m_nSceneID == -1 then
		self.m_nSceneID = nSceneID
		return _ObjEnterScene(self.id, nSceneID, nX, nY) 
	end
end

function Obj:LeaveScene()
    if self.m_nSceneID ~= -1 then
        local nSceneID = self.m_nSceneID
        self.m_nSceneID = -1
        return _ObjLeaveScene(self.id, nSceneID)
    end
    return false
end

function Obj:SetReady()
    return _SetObjReady(self.id)
end


function Obj:GetObjType()
    return _GetObjType(self.id)
end

function Obj:GetPosition()
    return _GetPosition(self.id)
end

function Obj:ChangePosition(nX, nY)
    return _ChangePosition(self.id, nX, nY)    
end

function Obj:SetEngineMoveSpeed(nSpeed)
    return _SetMoveSpeed(self.id, nSpeed)
end

function Obj:UpdateCharCacheAttr(nAttr, nValue)
	return _UpdateCharCacheAttr(self.id, nAttr, nValue);
end

function Obj:SetDiaphaneity(nDiaphaneity)
    if nDiaphaneity < 0 or nDiaphaneity > 100 then
        print("error: invalid diaphaneity value ", nDiaphaneity)
        return
    end

    if self:GetObjType() ~= CommonDefine.OBJ_TYPE_HUMAN then
        print("error: set diaphaneity not allowed", nDiaphaneity)
        return
    end

	_SetDiaphaneity(self.id, nDiaphaneity);

    local gcSetDiaphaneity = Dispatcher.ProtoContainer[PacketID.GC_SET_DIAPHANEITY] 
    gcSetDiaphaneity.objId = self.id
    gcSetDiaphaneity.diaphaneity = nDiaphaneity
    Msg.ZoneBroadCast(gcSetDiaphaneity, self.id)
end

function Obj:SetHide()
    self:SetDiaphaneity(0)
end

function Obj:SetDisplay()
    self:SetDiaphaneity(100)
end

function Obj:MoveTo(x, y,sendToSelf)
    if sendToSelf then
        _MoveTo(self.id,x,y, 1);
    else
        _MoveTo(self.id,x,y, 0);
    end
end

function Obj:IsMoving()
	return _IsMoving(self.id);
end

function Obj:GetDistance( nTargetId)
	return _GetDistance(self.id, nTargetId);
end

function Obj:ScanHuman(nX, nY, nRadius)
	return _ScanHuman(self.id, nX, nY, nRadius)
end

function Obj:ScanHumanAndMonster(nX, nY, nRadius)
	return _ScanHumanAndMonster(self.id, nX, nY, nRadius)
end

function Obj:IsInValidRadius(nX, nY, iRadius)
	local iSelfX, iSelfY = _GetPosition(self.id)
	
	local iDistX = nX - iSelfX
	local iDistY = nY - iSelfY
	
	return iRadius * iRadius >= iDistX * iDistX + iDistY * iDistY
end

function Obj:ReleaseObj()
	-- 干掉这个obj对应的所有的timer
    for i = #TimeEvents, 1, -1 do
		local node = TimeEvents[i]
		if node.objID == self.id then
			node.maxTimes = 0
		end
    end

	_ReleaseObj(self.id)
end

function Obj:CreateObj(nObjType, nFD)
	return _CreateObj(nObjType, nFD)
end

function Obj:CleanFD()
	_CleanFD(self.id)
end

function Obj:IsInJumpArea(nObjID)
    return _IsInJumpArea(nObjID)
end

function Obj:IsInBuffArea()
	return _IsInBuffArea(self.id)
end


function Obj:IsObjInSameZone(nObjID)
	return _IsObjInSameZone(self.id, nObjID)
end

function Obj:SendDieMsg(oKiller)
	if self:GetObjType() ~= CommonDefine.OBJ_TYPE_HUMAN and
       self:GetObjType() ~= CommonDefine.OBJ_TYPE_MONSTER and
       self:GetObjType() ~= CommonDefine.OBJ_TYPE_PET then
		return
	end
	
	local oObjDieMsg = Dispatcher.ProtoContainer[PacketID.GC_OBJ_DIE]
	oObjDieMsg.objId = self.id
	oObjDieMsg.objType = self:GetObjType()
	oObjDieMsg.objKillerId = oKiller.id
	oObjDieMsg.objKillerType = oKiller:GetObjType()
	Msg.ZoneBroadCast(oObjDieMsg, self.id)
end

function Obj:IncTimerID(nTimerID, nCount)
	self.m_oTimerIDs[nTimerID] = nCount
end
]]--
