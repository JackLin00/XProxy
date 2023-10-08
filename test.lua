

-- local r = requests.get("https://www.baidu.com")
-- print(r.header)

conf.set("host", "127.0.0.1", "common")
-- conf.set("port", 13145, "common")


function OnXProxycLogin(data)
    print(data)
    return 0
end

function AfterXProxycLogin(info, bind_data)
    print(info)
    print(bind_data)
end