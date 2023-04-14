//
// Created by lrsoft on 2023/4/12.
//

#include <vector>
#include <sstream>
#include "RequestParser.hpp"
#include "Request.hpp"
#include "../Utils/UrlUtils.h"

RequestParser::ResultEnum RequestParser::parseRequestItem(Request &req, char input) {
    // 开头第一行GET / HTTP/1.1
    switch (_state) {
        case method_start:
            // 啥都不是
            if (!isChar(input) || isCtl(input) || isTspecial(input)) {
                return bad;
            } else {
                // 读GET POST之类的方法阶段
                _state = method;
                req.method.push_back(input);
                return indeterminate;
            }
        case method:
            // 方法名后你最好是空格
            if (input == ' ') {
                _state = uri;
                return indeterminate;
            } else if (!isChar(input) || isCtl(input) || isTspecial(input)) {
                return bad;
            } else {
                req.method.push_back(input);
                return indeterminate;
            }
        case uri:
            // 没url那就跳阶段读主版本
            if (input == ' ') {
                _state = http_version_h;
                return indeterminate;
            } else if (isCtl(input)) {
                return bad;
            } else {
                // 读url
                req.uri.push_back(input);
                return indeterminate;
            }
        case http_version_h:
            // 下面拼一个HTTP出来
            if (input == 'H') {
                _state = http_version_t_1;
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_t_1:
            if (input == 'T') {
                _state = http_version_t_2;
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_t_2:
            if (input == 'T') {
                _state = http_version_p;
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_p:
            if (input == 'P') {
                _state = http_version_slash;
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_slash:
            // 分隔符后面版本号比如1.1
            if (input == '/') {
                req.http_version_major = 0;
                req.http_version_minor = 0;
                _state = http_version_major_start;
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_major_start:
            if (isDigit(input)) {
                req.http_version_major = req.http_version_major * 10 + input - '0';
                _state = http_version_major;
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_major:
            // 读到.就是主版本结束了，跳去读次版本
            if (input == '.') {
                _state = http_version_minor_start;
                return indeterminate;
            } else if (isDigit(input)) {
                // 是主版本的个位，把原个位变十位+现在的值
                req.http_version_major = req.http_version_major * 10 + input - '0';
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_minor_start:
            if (isDigit(input)) {
                // 读次版本号
                req.http_version_minor = req.http_version_minor * 10 + input - '0';
                _state = http_version_minor;
                return indeterminate;
            } else {
                return bad;
            }
        case http_version_minor:
            // 次版本结束了换行
            if (input == '\r') {
                _state = expecting_newline_1;
                return indeterminate;
            } else if (isDigit(input)) {
                // 次版本原先的值x10+现在的
                req.http_version_minor = req.http_version_minor * 10 + input - '0';
                return indeterminate;
            } else {
                return bad;
            }
        case expecting_newline_1:
            // 第一行结束就应该换行
            if (input == '\n') {
                _state = header_line_start;
                return indeterminate;
            } else {
                return bad;
            }
        case header_line_start:
            // 换行后回车
            if (input == '\r') {
                _state = expecting_newline_3;
                return indeterminate;
            } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
                _state = header_lws;
                return indeterminate;
            } else if (!isChar(input) || isCtl(input) || isTspecial(input)) {
                return bad;
            } else {
                req.headers.push_back(Header());
                req.headers.back().name.push_back(input);
                _state = header_name;
                return indeterminate;
            }
        case header_lws:
            if (input == '\r') {
                _state = expecting_newline_2;
                return indeterminate;
            } else if (input == ' ' || input == '\t') {
                return indeterminate;
            } else if (isCtl(input)) {
                return bad;
            } else {
                _state = header_value;
                req.headers.back().value.push_back(input);
                return indeterminate;
            }
        case header_name:
            if (input == ':') {
                _state = space_before_header_value;
                return indeterminate;
            } else if (!isChar(input) || isCtl(input) || isTspecial(input)) {
                return bad;
            } else {
                req.headers.back().name.push_back(input);
                return indeterminate;
            }
        case space_before_header_value:
            if (input == ' ') {
                _state = header_value;
                return indeterminate;
            } else {
                return bad;
            }
        case header_value:
            if (input == '\r') {
                _state = expecting_newline_2;
                return indeterminate;
            } else if (isCtl(input)) {
                return bad;
            } else {
                req.headers.back().value.push_back(input);
                return indeterminate;
            }
        case expecting_newline_2:
            if (input == '\n') {
                _state = header_line_start;
                return indeterminate;
            } else {
                return bad;
            }
        case expecting_newline_3:
            return (input == '\n') ? good : bad;
        default:
            return bad;
    }
}

bool RequestParser::isChar(int c) {
    return c >= 0 && c <= 127;
}

bool RequestParser::isCtl(int c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

bool RequestParser::isTspecial(int c) {
    switch (c)
    {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
            return true;
        default:
            return false;
    }
}

bool RequestParser::isDigit(int c) {
    return c >= '0' && c <= '9';
}

RequestParser::RequestParser(): _state(method_start) {}

void RequestParser::reset() {
    _state = method_start;
}

RequestParser::ResultEnum RequestParser::parseForm(Request &request, std::stringstream &stream) {
    // 把headers全读取到Map
    RequestParser::ResultEnum result = bad;

    // 读取str里的内容，记录Content-Type: 后面的内容到request.contentType
    // 当检测到Content-Length: 开头的文字后，下面的数字是内容长度
    std::string line;
    // 读content类型与长度
    while (std::getline(stream, line)) {
        if (line.find("Content-Type: ") == 0) {
            request.contentType = line.substr(14);
        } else if (line.find("Content-Length: ") == 0) {
            // 读完后面就没必要循环了
            request.contentLength = std::stoi(line.substr(16));
            result = good;
            break;
        }
    }

    // 有了content类型和长度就开始解析content
    if(result == good) {
        // 读当前行content
        std::cout << "contentLength: " << request.contentLength << std::endl;
        std::cout << "streamTellG: " << stream.tellg() << std::endl;
        std::cout << "streamLength: " << stream.str().length()<< std::endl;

        // 如果当前流的长度小于contentLength+2（\r\n），说明还没读完
        if(request.contentLength + stream.tellg() > stream.str().length()) {
            std::cout << "need read more" << std::endl;
            return indeterminate;
        }

        // 解析-xform，格式username=123456&password=aaaaa&email=asdf%40sdf.sadf
        if(request.contentType.find("application/x-www-form-urlencoded") == 0){
            // x-form开头\r\n并不会算进contentLength里，所以要先读两个
            stream.get();
            stream.get();

            std::string content;
            content.resize(request.contentLength);
            stream.read(&content[0], request.contentLength);

            // 解析urlencoded
            std::string key, value;
            bool isKey = true;
            for (auto &c : content) {
                if (c == '=') {
                    isKey = false;
                } else if (c == '&') {
                    auto decodeKey = UrlUtils::urlDecode(key);
                    auto decodeValue = UrlUtils::urlDecode(value);
                    request.contentMap[decodeKey] = decodeValue;
                    key.clear();
                    value.clear();
                    isKey = true;
                } else {
                    if (isKey) {
                        key.push_back(c);
                    } else {
                        value.push_back(c);
                    }
                }
            }
            if (!key.empty()) {
                auto decodeKey = UrlUtils::urlDecode(key);
                auto decodeValue = UrlUtils::urlDecode(value);
                request.contentMap[decodeKey] = decodeValue;
            }

        }else if(request.contentType.starts_with("multipart/form-data")){

        }

        // header进map
        for (auto &header : request.headers) {
            request.headerMap[header.name] = header.value;
        }
        // 卸磨杀驴
        request.headers.clear();
    }
    return result;
}
