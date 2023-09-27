local haha1 = {1,2,3,4,5}
haha1["xxxx"] = "jacklin"
haha1["xxxxq"] = {4, 5, 6}
local haha2 = {jack = "dasda"}
print("in lua script")
print(haha1)
-- print(haha["jakcl"])
local xxxxx = cjson.decode('{"info":[{"index":0,"local_ip":"127.0.0.1","name":"SSH","port":22},{"index":1,"local_ip":"127.0.0.1","name":"WEB","port":80}]}')

print(xxxxx)
-- -- print(haha)
-- print(haha1)
-- print(haha2)

function OnXProxycLogin(data)
    print(data)
    return 1
end