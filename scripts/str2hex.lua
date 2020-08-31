local Util = require("common.Util")
sp = require("common.Util").scriptPath


function extractQiangda()
    wf = io.open("qiangdaDict.txt","w")
    for line in io.lines("qiangda.txt") do
        --[[
        local t =  {}
        for k, v in string.gmatch(line, "([%z\1-\127\194-\244][\128-\191]*)%s+(%w+)") do
            t[k] = v
        end
        Util.PrintTable(t)
        ]]--
        --print(line)
       --[[ local lastSpace=  line:match'^.*()%s'
        q[1] = line:sub(1,lastSpace)
        q[2] = line:sub(lastSpace+1)
        ]]--
        local q = {}
        local q = Util.string.split(line,"  ")

        if  q[2] == nil or string.mpack(q[2]) == "D" then 
            print("no 2rd")
            for match in line:gmatch("(.-)".."%s".."([^%s.*])") do
                print("match:"..match..",len:"..match:len())
                q[1] = match
                
                q[2] = string.split(line:sub(match:len(),-1)," ",1)
          --      local ls = q[2][1]:match'^.*()%s'
           --     q[2][1] = q[2][1]:sub(ls+1)
                local tmp = string.mpack(q[2][1])
                print(tmp)
                local sep = "99"

                local lastIndex = nil
                local p = string.find(tmp, sep, 1)
                lastIndex = p
                while p do
                    p = string.find(tmp, sep, p + 1)
                    if p then
                        lastIndex = p
                    end
                end
                if not (lastIndex== nil) then
                    print(lastIndex,tmp:len())
                    if (tmp:len() - lastIndex == 4) and (tmp:sub(-3,-2)>="41" and (tmp:sub(-3,-2) <="44")) then
                        print('section:'..Util.hex2string(tmp:sub(1,lastIndex-1)))
                        print('answer:'..Util.hex2string(tmp:sub(-3,-2)))
                        q[1] = q[1]..Util.hex2string(tmp:sub(1,lastIndex-1))
                        q[2]= Util.hex2string(tmp:sub(-3,-2))
                    end
                else
                end
           --[[
                if tmp:sub(1,3) == "99E" then
                    tmp = tmp:sub(3)
                end
                q[2][1] = Util.hex2string(tmp)
                if q[2][1]:sub(-1) == "D" then
                    print('lat char:'..q[2][1]:sub(-1))
                    --q[2][1] = q[2][1]:sub(1,-2)
                end
                ]]--
                break
            end
        end
        Util.PrintTable(q)
        if not (q[2]==nil) then
            if type(q[2]) == "table" then
                wf:write(q[1].."|"..q[2][1].."\n")
            else
                wf:write(q[1].."|"..q[2].."\n")
            end
        end
    end
    wf:close()

end

function extractXiaohua()
    wf = io.open("xiaohuaDict.txt","w")
    for line in io.lines("xiaohua.txt") do
        local ct = line:sub(line:find(" ")+1,-1) 
        print(ct)
        wf:write(ct.."\n")
    end
    wf:close()
end

function extractDialog(fileName)
    local dialog = Util.extractDict(fileName)
    print('lines:'..#dialog)
    wf = io.open("girlDialog.txt","w")
    for k,v in ipairs(dialog) do 
        wf:write(v)
        --print(v)
    end
    io.close(wf)
end

function encodeFile(fileName,outFile)
    wf = io.open(outFile,"ab")
    local dt = {}
--    local preLine = ""
    for line in io.lines(fileName) do 
        line = string.trim(line)
        local ecLine = string.mpack(line)
        table.insert(dt,ecLine)
 --       preLine = line
    end
    table.sort(dt)
    for k,v in ipairs(dt) do
        wf:write(v.."\n") 
    end
    io.close(wf)
end

function decodeFile(fileName)
    wf = io.open('cdict_s.txt',"a")
    for line in io.lines(fileName) do
       local dcLine = Util.hex2string(line)
       --print(dcLine)
       wf:write(dcLine.."\n")
    end
    io.close(wf)
end

function gzipFile()
    local zlib = require "zlib"
    local stream = zlib.deflate()
    local rf = assert(io.open("ecdict.txt","r") or " file not found ecdict.txt")
    local wf = io.open("ecdict.gzip","a")
    for line in io.read()  do
        local zipped = stream(line)
        wf:write(zipped)
    end

    io.close(wf)
    io.close(rf)
end

function firstLetter(str)
    return str:match("[%z\1-\127\194-\244][\128-\191]*")
end

--將不是 成語|解釋的行刪除
function fixdoc(fileName)
    local ccc = string.mpack("|")
    local i = 0
    local preline = ""
    for line in io.lines(fileName) do
        local ml = string.mpack(line)
        --print(ml:sub(25,26))
        local cc = ml:sub(25,26)
        if not  (cc == ccc) then
            print(preline)
            print(line)
        else
            preline = line
            --print(ml:sub(25,26))
        end
    end
    io.close()
end

--[[
local ecstr = string.mpack("一心一意|,.。")
print("encoded:"..ecstr)
local dcstr = Util.hex2string(ecstr)
print("decoded:"..dcstr)
]]--
extractQiangda()
--extractXiaohua()
--extractDialog("girlDict2.4.txt")
--gzipFile()
--decodeFile("ecdict.txt")
--encodeFile("cidict.txt","ecdict.txt")
--fixdoc("cidict.txt")
--print(Util.hex2string("99E"))
