//
// Created by lrsoft on 2023/4/10.
//

#ifndef SERVERFRAME_LISTENER_H
#define SERVERFRAME_LISTENER_H

#include <atomic>
#include <memory>
#include <string>
#include <asio/ssl.hpp>
#include <lua.hpp>
#include "SimpleAsioDefine.h"
#include "Utils/Timer.h"


class Listener {
private:
    typedef asio::ip::tcp::resolver AsioResolver;
public:
    Listener(const std::string & name, lua_State* state);
    void init();
    void join();

    bool isTerminated() const { return _isTerminated; }

private:
    void loadSettings();
    // 异步新连接
    void doAccept();
    // 异步等信号
    void doAwaitStop();

private:
    std::atomic<bool> _isTerminated = false;
    bool _isHttps = false, _hasUrl = false, _hasInterceptor = false;
    int _port = -1;

    std::string _address, _appName, _staticFolder;
    std::string _certPath, _keyPath;
    std::shared_ptr<asio::ssl::context> _pContext;

    AsioService _service;
    AsioAcceptor _acceptor;

    lua_State* _pState;

    // 终止信息
    asio::signal_set _signals;
};


#endif //SERVERFRAME_LISTENER_H
