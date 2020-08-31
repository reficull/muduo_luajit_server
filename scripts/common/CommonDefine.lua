module(...,package.seeall)

OBJ_TYPE_INVALID = -1			--// 无效
OBJ_TYPE_HUMAN = 0	            --// 玩家

-- 断开连接错误码
DISCONNECT_REASON_ANOTHER_CHAR_LOGIN = 1                -- 角色在其它地方上线
DISCONNECT_REASON_CHANGE_TO_CROSS_SCENE = 2             -- 角色从游戏服切换到跨服副本服 游戏服断开连接
DISCONNECT_REASON_REDIERCT_TO_CROSS_SCENE = 3   -- 角色正在跨服副本中，重定向到跨服副本，游戏服断开连接
DISCONNECT_REASON_ADMIN_KICK = 4                                -- 管理后台踢人
DISCONNECT_REASON_SERVER_FULL = 5                               -- 服务器人满
DISCONNECT_REASON_CROSS_ACCOUNT_ERR = 50                -- 错误帐号（登录中间服）
-- 100 开始是c++层的错误码
DISCONNECT_REASON_CLIENT = 100                                  -- client主动断开
DISCONNECT_REASON_TIMEOUT = 101                                 -- 长时间没有发包断开
DISCONNECT_REASON_PACKET_ERR = 102                              -- 发送非法包断开
-- 登录错误码
ASK_LOGIN_OK = 1                -- 成功登录
ASK_LOGIN_ERROR_CREATE_CHAR = 2 -- 登录失败 创建角色失败
ASK_LOGIN_GO_TO_MSVR = 3        -- 跨服pk中，需要重定向到msvr
ASK_LOGIN_SERVER_FULL = 4       -- 服务器人数已满 无法登录
ASK_LOGIN_CHARACTER_NONEXISTS = 5 -- 人物不存在

REGISTER_OK = 1
REGISTER_ACCOUNT_EXISTS = 2

CREATE_ROLE_OK = 1
CREATE_ROLE_NAME_EXISTS = 2
