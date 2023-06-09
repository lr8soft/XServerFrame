local httpApp = require("app/httpServer")
local httpsApp = require("app/httpsServer")

manage = {
    -- listener入口
    app = {
        httpServerDemo = httpApp,
        httpsServerDemo = httpsApp
    }
}

-- 代理访问
function manage.callUrlMethod(appStr, urlStr, req)
    local fullFunc = "manage.app." .. appStr .. ".url." .. urlStr
    print("invoke" .. fullFunc)
    local func = _G
    for w in string.gmatch(fullFunc, "[%w_]+") do
        func = func[w]
    end

    if type(func) ~= "function" then
        error("Invalid function name: " .. fullFunc)
    end

    
    -- 调用拦截器
    local interceptor = manage.app[appStr]["interceptor"]
    if interceptor ~= nil then
        if type(interceptor) == "function" then
            interceptorResponse = interceptor(req)
            if interceptorResponse ~= nil then
                return interceptorResponse
            end
        end
    end


    -- 没拦截的话就调用方法
    return func(req)
end


return manage