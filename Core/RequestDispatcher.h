//
// Created by lrsoft on 2023/4/13.
//

#ifndef SERVERFRAME_REQUESTDISPATCHER_H
#define SERVERFRAME_REQUESTDISPATCHER_H
#include <map>
#include <memory>
#include <string>
#include "../Resolvers/IRequestSolver.h"

class RequestDispatcher {
public:
    static RequestDispatcher* getInstance(){
        if(pInst == nullptr)
            pInst = new RequestDispatcher;
        return pInst;
    }

    void addHandler(const std::string method, std::shared_ptr<IRequestSolver> handler);
    void handleRequest(const Request& req, Reply& rep);
private:
    static RequestDispatcher* pInst;

    // METHOD => Requesthandler
    std::multimap<std::string, std::shared_ptr<IRequestSolver>> handleGroup;
};


#endif //SERVERFRAME_REQUESTDISPATCHER_H