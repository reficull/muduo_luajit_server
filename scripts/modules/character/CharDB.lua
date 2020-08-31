module(..., package.seeall)

-- char db， 需要保存到数据库的信息
--             CharDB 都要new出来
-- 用hash保存userinfo  multi_hget,multi_hset
-- name 也是唯一，是这个游戏 唯一名，account是神迹通行证的账号名
-- 在设置name时，写入hash : account_name, key为account,val为name，用以关联user的zset
-- 以及 name_account, key为name,val 为account
-- list: 用DB:zset("char","account",1),DB:zget("char","account")

local PrintT = require("common.Util").PrintTable
local ns = "char"                --数据库表
local CommonDefine = require("common.CommonDefine")
CharDB={}
--local MailManager = require("mail.MailManager")
local Util = require("common.Util")
function CharDB:defaultKV()
    local kv={
        svrname=SVRNAME,
        account="",
        password="",
        name="",
        lv=1,hp=0,mp=0,mapid=101,
        exp=0,lingli=0,rmb=10000,bindrmb=5000,money=20000,bindmoney=20000,billmoney=40000,zhandouli=0,
        job=0,x=0,y=0,prevMapid=101,prevX=0,prevY=0,
       -- realName="",idCard="",
        fcm=0,olTime=0,lastDate=os.time(),lastLogin=os.time(),lastLogout=os.time(),
        escortType=1,insurance=0,escortStatus=nil,escorttime=nil,escortRescue=nil,shipType=1,
        notifyPhrase=0,
        gender=0,
        head=1,
        --noTip={},
        bagCap = 72,
        storeCap = 72,
        pkvalue = 0,
        --skills={},
        --cooldowns={},
        --buffers={},
        --toolbarShortcuts={},
        --taskRecord = {},
        --taskTimes = {},
        --taskCallback = {},
        --sceneTimes = {},        --副本进入次数
        --bag={},
        --store = {},
        --bagEquip = {},
		--blacks = {},			-- 最近联系人
        --guildList ={},			--帮会邀请列表
        --magicBox = {},
        onLineDay = 1,              --连续登录天数
        loginRewardRecord = 0,      --连续登录记录
        currentActivity = 0,         --当前活跃度
        --activityPresentGetList = {},  --活动礼包领取列表
        charm = 0,--总魅力值
        todayCharm = 0, --今天魅力值
        receFlower = 0,--收到鲜花总数
        receFlowerToday = 0, --今天收到鲜花数
        sendFlower = 0,--送出鲜花总数
        sendFlowerToday = 0, --今天送出鲜花数
        weaponCooldownTime = 0,   --武器修炼冷确时间
        syssetting = 1,       --系统设置
        vipPetNum = 0, --VIP宠物格
        --pet={},       --宠物数组
        --horse = {},   --坐骑数组
        --illustration = {}, -- 图鉴数据
        forbidChatTime=0,     --玩家禁言结束时间
        activity = {"activity1","activity2"},        --活动记录
        ip = "",              --玩家登陆ip地址
    }
    return kv 

end
-- 这里的表还是显示的初始化比较好 否则db数据格式升级后 table里可能会有nil对象
function CharDB:New()
    print("CharDB:New called")
    local o = CharDB:defaultKV()
    setmetatable(o,self)
    self.__index=self
    return o;
end 

-- 直接查询db 帐号是否存在
function IsNameExistInDB(name,checkAccount)
    print("check name exists:",name)
    --Util.PrintTable(name)
    local char
    if checkAccount then
        char = DB:hget("account_name",name)
    else
        char = DB:hget("name_account",name)
    end
    if char == "" then
        return false
    else
        return true
    end
end
-- 允许已经有account的存在但rolename为空
function CharDB:Add()
    -- test 是否存在
    if IsNameExistInDB(self.name) then
        return false,"用戶已經存在"
    end
    if not self.account then
        return false,"没有账号名"
    end
    DB:hset("account_name",self.account,self.name)
    DB:hset("name_account",self.name,self.account)
    return DB:multi_hset("char:"..self.account,self)
    -- 加入列表l_char:USER:xxx，以及hash USER:xxx
     
end

function CharDB:Save()
    --update
    DB:multi_hset("char:"..self.account,self)
end

-- 直接查询db 获取离线角色的特定属性
function GetCharPropertyOffLine(name, key)
    local val = DB:hget("char:"..self.name,key)
    if not val then
        return nil
    end
	
	return val 
end

-- 直接查询db 修改离线角色的特定属性
function SetCharPropertyOffLine(name, kvs)
    local account = DB:hget("name_account",name)
    if not account then
        return false
    end
    local tb = DB:hgetall("char:"..account)
    if not tb then
        return nil
    end
    for k,v in ipairs(kvs) do
        tb[k] = v
    end
	return DB:multi_hset("char:"..account,tb)	
end

function CharDB:LoadByName(name)
    local account = DB:hget("name_account",name)
    if not account then
        return false
    end
    return CharDB:LoadByAccount(account)
end

function CharDB:LoadByAccount(account)
    local tb = DB:hgetall("char:"..account)
    if table.isEmpty(tb) then
        return false
    end
    for k,v in pairs(tb) do
        self[k] = v
    end
    return true;
end

function CharDB:Save()
    return DB:multi_hset("char:"..self.account,self)
end


--同步实时更新接口
function CharDB:SynchSave(key)
	if self[key] == nil then
		return
	end
	return DB:hset("char:"..self.account,key,self[key])
end

function CharDB:ReSetMetatable()
    CharDB.__index = CharDB;
    setmetatable(self, CharDB);
end

