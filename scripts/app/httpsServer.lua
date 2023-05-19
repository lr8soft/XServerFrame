local userModule = require("app/user")
httpsServer = {
    isHttps = true,
    port = 443,
    address = "0.0.0.0",
    -- 静态内容文件夹，如html、css、js、图片等
    static = "statics",
    -- HTTPS证书
    cert = {
        cert = "cert/localhost.crt",
        key = "cert/localhost.key"
    },
    url = {
        user = userModule
    },

    interceptor = function(req)
        print("HTTPS Request Detected! Host:", req.HEADER.Host)
        -- 返回nil即不拦截
        return nil
    end
}

return httpsServer