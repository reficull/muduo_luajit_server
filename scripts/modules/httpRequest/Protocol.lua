module(..., package.seeall)

CGGet = {
    {"URL","CHAR",255}, -- 請求地址
    {"BODY","CHAR",255},
    {"TYPE","INT",1}, -- 返回格式 1為gzip
}

GCGet = {
    {"CONT","CHAR",255}
}


