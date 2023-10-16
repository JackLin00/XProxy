

-- local r = requests.get("https://www.baidu.com")
-- print(r.header)




-- local set_headers = {jack = "123"}
-- local data = {aka = 12312, data = "dasdasda"}
-- local xparams = {data = 3.14, xxx = "dasdas", bbb = 1111}
-- -- requests.post("http://127.0.0.1:8889/device_change?aa=1.1&xx=das", {headers = set_headers, json = data, params = xparams})
-- requests.get("http://127.0.0.1:8889/get_devices?aa=1.1&xx=das", {headers = set_headers, params = xparams})
-- requests.post("http://127.0.0.1:8889/device_change", {headers = set_headers, json = data, params = xparams})


function OnXProxycLogin(data)
    -- print(data)
    return 0
end

function OnXProxycLoginSucc(data)
    data["act"] = "add"
    print(data)
    rsp = requests.post("http://127.0.0.1:8889/device_change", {json = data})
    print(rsp.status_code)
end

function OnXProxyLoginOut(data)
    print("close data : ", data)
    data["act"] = "del"
    rsp = requests.post("http://127.0.0.1:8889/device_change", {json = data})
    print(rsp.status_code)
end
