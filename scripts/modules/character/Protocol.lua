module(..., package.seeall)

CGGetName = {
    {"gender","INT",    1}
}

GCGetName = {
    {"name","CHAR",    32}
}
CGCreateRole = {
    {"career", "INT", 1},
    {"gender",  "INT",  1},
    {"accname",   "CHAR",  32},
    {"rolename",   "CHAR",  32},
}
--[10001,["asdf","asdf",23232323]]
CGAskLogin = { 
    {"account",     "CHAR",      32},    -- 帐号
    {"password",     "CHAR",      32},    -- 帐号
    {"timestamp",   "INT",       1},     -- 时间戳
    {"authkey",     "CHAR",      128},   -- 校验key
}
GCAskLogin = { 
    {"result",     "SHORT",      1},     -- 登录结果
    {"svrName",    "CHAR",       32},    -- 游戏服务器名称
    {"msvrIP",     "CHAR",       32},    -- 跨服pk服ip
    {"msvrPort",   "INT",        1},     -- 跨服pk服port
}
CGRegister = {
    {"account",     "CHAR",     32},
    {"password",     "CHAR",      32},    -- 帐号
    {"timestamp",   "INT",       1},     -- 时间戳
    {"authkey",     "CHAR",      128},   -- 校验key
}
GCRegister = {
    {"status",      "INT",      1},
}
GCHumanInfo = {
    {"rolename",   "CHAR",       64}, -- 角色名字
    {"job",        "SHORT",      1},  -- 角色职业
    {"gender",     "SHORT",       1},  -- 角色性别
    {"level",      "INT",        1},  -- 级别
    {"speed",      "SHORT",      1},  -- 移动速度
    {"atkmode",    "CHAR",       1},  -- 攻击模式
    {"zhenyin",    "CHAR",         32},  -- 阵营
    {"guild",    "CHAR",         32},  -- 帮派
    {"timeServer", "INT",        1},  -- 服务器当前时间
    {"objid",      "INT",        1},  -- ObjID
    {"mapid",      "SHORT",      1},  -- 地图id
    {"maptype",    "CHAR",       1},  -- 地图类型
    {"scenex",     "INT",        1},  -- 场景x坐标
    {"sceney",     "INT",        1},  -- 场景y坐标
    {"mode",       "CHAR",       1},  -- 白天/黑夜模式
    {"mapWidth", "SHORT", 1},
    {"mapHeight", "SHORT", 1},
    {"bodyid",    "SHORT",      1},   --bodyid
    {"weapon",    "SHORT",      1},   --武器id
    {"signature",    "CHAR",      64},   --个性签名
}
                                
--[[
CGCreateRole = { 
    {"gender",  "INT",  1}, 
    {"accname",   "CHAR",  32},
    {"rolename",   "CHAR",  32},
}
--]]
GCCreateRole = {
    {"status", "INT", 1},
}

CGDisconnect = {
    {"status","INT",1}
}
