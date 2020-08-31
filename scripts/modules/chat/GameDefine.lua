module(...,package.seeall)

-- 名稱, 開始指令，結束指令
GAMES = {
    chengyu ={
        name = 'chengyu',
        startCmd ={ 'chengyu','cy','成语接龙','龙虫成语'},
        stopCmd = {'chengyuend','cyend','成语接龙结束','结束成语接龙'},
        timeLimit = 180
    },
    huaquan = {
        name = 'huaquan',
        startCmd = {'huaquan','划拳'},
        stopCmd = {'huaquanend','划拳结束'},
    },
    qiangda = {
        name = "qiangda",
        startCmd = {"qiangda","qd","抢答"},
        stopCmd = {"stopqianda","eq"},
    },
}
