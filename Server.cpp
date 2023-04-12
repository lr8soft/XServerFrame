//
// Created by lrsoft on 2023/4/10.
//

#include <iostream>
#include "Server.h"
#include "Utils/LogUtil.h"
#include "Utils/HttpConnect.h"

Server::Server(AsioService &service, const std::string & addr, const std::string & port)
    : _service(service), _acceptor(service){
    AsioResolver resolver(_service);
    AsioEndPoint endPoint = *resolver.resolve({addr, port});
    _acceptor.open(endPoint.protocol());
    _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(endPoint);
    _acceptor.listen();
    this->start();
}

void Server::start() {
    std::shared_ptr<AsioSocket> pSocket(new AsioSocket(_service));
    _acceptor.async_accept(*pSocket, std::bind(&Server::handleAcceptAction, this, pSocket));
}

void Server::handleAcceptAction(std::shared_ptr<AsioSocket> pSocket) {
    std::memset(buf, 0, sizeof(buf));
    pSocket->async_read_some(asio::buffer(buf, std::strlen(buf)),
                             std::bind(&Server::handleRequest, this, pSocket));
    std::cout << "handleAcceptAction buf " << buf << " " <<pSocket->remote_endpoint().address() <<std::endl;
    this->start();
}

void Server::handleRequest(std::shared_ptr<AsioSocket> pSocket) {
    std::memset(buf, 0, sizeof(buf));
    pSocket->async_receive(asio::buffer(buf, std::strlen(buf)),
                             std::bind(&Server::handleReceiveRequest, this, pSocket));
    std::cout << "handleRequest buf " << buf << " " << pSocket->remote_endpoint().address() << std::endl;
}

void Server::handleReceiveRequest(std::shared_ptr<AsioSocket> pSocket) {
    char result[] = "HTTP/1.1 200 OK\n"
                    "Content-Type: text/html\n"
                    "Content-Length: 1024\n"
                    "Server: ServerFrame v0.0\n"
                    "\n"
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "    <title>Example</title>\n"
                    "</head>\n"
                    "<body>\n"
                    "    <h1>Hello, world!</h1>\n"
                    "</body>\n"
                    "</html>";
    pSocket->async_write_some(asio::buffer(result, strlen(result)),
                              std::bind(&Server::handleReleaseSocket, this, pSocket));
    std::cout << "handleReceiveRequest " << pSocket->remote_endpoint().address() << std::endl;
}

void Server::handleReleaseSocket(std::shared_ptr<AsioSocket> pSocket) {
    std::cout << "Release " << pSocket->remote_endpoint().address() << std::endl;
    pSocket->release();
}
