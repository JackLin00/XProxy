

-- local r = requests.get("https://www.baidu.com")
-- print(r.header)

local table1 = {aa = 1223, bb = 2222, cc = 33333, kk = "dasdsa"}
local table2 = {aa = 1223, bb = 2222, cc = 33333, dd = {aa = 111, eee = 2222}}
local table3 = {1, 2, 3, 4, 5}
print(table1, 12321)
print(table2)
print(table3)



function OnXProxycLogin(data)
    print(data)
    return 0
end

function AfterXProxycLogin(info, bind_data)
    print(info)
    print(bind_data)
end