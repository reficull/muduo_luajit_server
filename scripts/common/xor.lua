cipher = "10101111" -- must be eight digit binary number

--Returns the XOR of two binary numbers
function xor(a,b)
  local r = 0
  local f = math.floor
  for i = 0, 31 do
    local x = a / 2 + b / 2
    if x ~= f(x) then
      r = r + 2^i
    end
    a = f(a / 2)
    b = f(b / 2)
  end
  return r
end

--Changes a decimal to a binary number
function toBits(num)
    local t={}
    while num>0 do
        rest=math.fmod(num,2)
        t[#t+1]=rest
        num=(num-rest)/2
    end
	--[[ t gives the binary number in reverse. To fix this
		the bits table will give the correct value
		by reversing the values in t.
		The result will be left paddied with zeros to eight digits
	]]
	local bits = {}
	local lpad = 8 - #t
	if lpad > 0 then
		for c = 1,lpad do table.insert(bits,0) end
	end
	-- Reverse the values in t
	for i = #t,1,-1 do table.insert(bits,t[i]) end

    return table.concat(bits)
end

--Changes eight digit binary to decimal
function toDec(bits)
	local bmap = {128,64,32,16,8,4,2,1} --binary map

	local bitt = {}
	for c in bits:gmatch(".") do table.insert(bitt,c) end

	local result = 0

	for i = 1,#bitt do
		if bitt[i] == "1" then result = result + bmap[i] end
	end

	return result
end

--Encryption and Decryption Algorithm for XOR Block cipher
function XORCODEC(str)
	--split cipher string into a table
	local ciphert = {}
	for c in cipher:gmatch(".") do table.insert(ciphert,c) end

	--split string into a table containing only binary numbers of each character
	local block = {}
	for ch in str:gmatch(".") do
		local c = toBits(string.byte(ch))
		table.insert(block,c)
	end

	--for each binary number perform xor transformation
	for i = 1,#block do
		local bitt = {}
		local bit = block[i]
		for c in bit:gmatch(".") do table.insert(bitt,c) end

		local result = {}
		for i = 1,8,1 do
			table.insert(result,xor(ciphert[i],bitt[i]))
		end

		block[i] = string.char(toDec(table.concat(result)))
	end

	return table.concat(block)
end
