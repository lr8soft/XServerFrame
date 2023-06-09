//
// Created by lrsoft on 2023/5/18.
//

#include "IConn.h"
#include "Reply.h"
#include "Request.hpp"
#include "RequestParser.hpp"
#include "../ConnManager.h"
#include "../Utils/LogUtil.h"
#include "RequestDispatcher.h"


void IConn::onReadAction(const std::error_code & code, std::size_t bytesCount, std::array<char, 8192> & _buffer,
                         std::stringstream & _bufferStream, Request & _request, Reply & _reply, RequestParser & _parser) {
    if(!code) {
        RequestParser::ResultEnum result;
        // 记录请求所有buffer
        _bufferStream.write(_buffer.data(), bytesCount);
        // get指针回到起始
        _bufferStream.seekg(0, std::ios::beg);

        // 解包解析结果
        result = _parser.parseHeader(_request, _bufferStream);
        // 解析成功
        if (result == RequestParser::good) {
            // 解析表单
            auto formResult = _parser.parseForm(_request, _bufferStream);
            // 只要不是没读完都行
            if(formResult != RequestParser::indeterminate) {
                RequestDispatcher::getInstance()->handleRequest(getAppName(), _request, _reply);
                LogUtil::printInfo(_request.method + ":" + _request.uri);
                // 清除缓存
                _bufferStream.clear();
                doWrite();
            }else {
                // 没读完记得清理解析了一半的request
                _request.reset();
                doRead();
            }
        } else if (result == RequestParser::bad) {
            //std::cout << "bad request " << _bufferStream.str() << std::endl;
            _reply = Reply::stockReply(Reply::bad_request);
            _bufferStream.clear();
            doWrite();
        } else {
            doRead();
        }

        // 记得重置解析器
        _parser.reset();
    }
    else if(code != asio::error::operation_aborted) {
        ConnManager::getInstance()->stopConn(shared_from_this());
    }
}
