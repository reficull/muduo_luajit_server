module(...,package.seeall)

--- chat type enum --
CHAT_TYPE_WORLD = 0
CHAT_TYPE_CAMP = 1
CHAT_TYPE_GUILD = 2
CHAT_TYPE_TEAM = 3
CHAT_TYPE_NEAR = 4
CHAT_TYPE_SPEAKER = 5


-- game type --
GAME_CHENGYU = 1

MENU_ALIAS = { 
   {'menu','菜单','#menu','#菜单'},
   {'games' ,'游戏菜单','#游戏菜单','游戏功能' },
}
MENUS = {
    'showMenu',
    'showGame',
}
    

MAIN_MENU = [[
#菜单,菜单,menu,#menu
╔═机器人菜单═╗
╚╗ 聊天管理 ╔╝
╔╝ 签到系统 ╚╗
╚╗ 游戏功能 ╔╝
╔╝ 便民工具 ╚╗
╚╗ 我的操作 ╔╝
╔╝ 帮派种植 ╚╗
╚╗ 群管功能 ╔╝
╠╝         ╚╣
╚═══════════╝
]]
GAME_MENU = [[
┏〓〓〓〓------游戏菜单---〓〓〓〓┓
●大小猜数 
●聚宝盆 
●字谜 -
●恶人谷 -
●一战到底 - 结束:#一战结束
●算24 -结束:#结束24
●芝麻 -
●整xx xx对方名字 - 
●摆珠 -
●彩票 -
●龙虫成语 -
●找你妹 -
●划拳 -
●花点大小x  x大或小  -
●三公x  x为赌注  -
●找茬  
]]
