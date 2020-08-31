local CharDB = require("character.CharDB").CharDB

--ObjHuman继承Obj的操作
ObjHuman = Obj:New()
OnlineAccountManager = OnlineAccountManager or {}
OnlineObjManager = OnlineObjManager or {}

--get object from online obj manager
function getOObj(nObjID)
    if OnlineObjManager[nObjID] == nil then
        local con = TCON(nObjID)
        OnlineObjManager[nObjID] = con
        return con
    else
        return OnlineObjManager[nObjID]
    end
end

function ObjHuman:ReSetMetatable()
    ObjHuman.__index = ObjHuman;
    self.m_db:ReSetMetatable();
    setmetatable(self, ObjHuman);
end

--get object from account manager
function ObjHuman:GetObj(nObjID)
    for k,v in pairs(OnlineAccountManager) do
        if v.id == nObjID then
            return v
        end
    end
    return nil
end

function ObjHuman:removeFromAccMng(nObjID)
    for k,v in pairs(OnlineAccountManager) do
        if v.id == nObjID then
            OnlineAccountManager[k]=nil
            return
        end
    end
    return nil
end
function ObjHuman:New(nObjID, roleName)
    --TODO see origin ObjHuman:New
    --create obj in c++ side,track user position
    if ObjManager[nObjID] then --已有用户创建新号
        local human = ObjManager[nObjID]
        human:Save()
        ObjManager[nObjID] = nil
    end

    if ObjManager[nObjID] == nil then
        ObjManager[nObjID] = {
            id = nObjID,
            roleName = roleName,
            m_db = CharDB:New(),
            --oBag = BagDB:New(180),
        }

    end

    local obj = ObjManager[nObjID]
    obj.offship = nil
    obj.isDisconnected = nil

    setmetatable(obj, self)
    self.__index = self

    return obj;

end

function ObjHuman:Load(username,bAccount)
    if bAccount then
        ret = self.m_db:LoadByAccount(username)
    else
        ret = self.m_db:LoadByName(username)
    end
    return ret
end

function ObjHuman:Destroy(nObjID)
    if nObjID == nil then
        OnlineAccountManager[self:GetAccount()] = nil
        ObjManager[self.id] = nil
        OnlineObjManager[self.id] = nil
        print("onLineAccountManager count:",#OnlineAccountManager)
    else
        obj = getOObj(nObjID)
        if obj ~= nil then
            print("conn:",obj)
            self:removeFromAccMng(nObjID)
            print("removed from account manager")
            ObjManager[obj] = nil
            print("removed from obj manager")
            OnlineObjManager[nObjID] = nil
            print("removed from online obj manager")
            print("remove " , nObjID ,"onLineAccountManager count:",#OnlineAccountManager)

        end
    end
end

function ObjHuman:AddUser()
    return self.m_db:Add()
end

function ObjHuman:Save()
    --self.m_db[key]=value
    local ret = self.m_db:Save()
end

function ObjHuman:DoDisconnect(nReason)
    self:Save()
    CppServer:handleLogout(self.id,nReason)
    self:Destroy()
end


function ObjHuman:GetName()
    return self.m_db.name
end

function ObjHuman:GetAccount()
    return self.m_db.account
end
function ObjHuman:GetGender()
    return self.m_db.gender
end

function ObjHuman:SetAccount(account)
    self.m_db.account = account
end

function ObjHuman:SetPassword(password)
    self.m_db.password=password 
end

function broadcast(msg)
    for k,v in pairs(OnlineObjManager) do
        if v ~= OnlineObjManager[k] then
            OnlineObjManager[k]:sendMsg("new user connected:")
        end
    end
end


