//
// Created by lrsoft on 2023/4/11.
//

#ifndef SERVERFRAME_CONNMANAGER_H
#define SERVERFRAME_CONNMANAGER_H

#include <set>
#include <memory>
#include "SimpleAsioDefine.h"
#include "Core/Connection.h"

class ConnManager {
public:
    static ConnManager* getInstance(){
        if(pInstance == nullptr)
            pInstance = new ConnManager;
        return pInstance;
    }

    void startConn(std::shared_ptr<Connection> pConn);
    void stopConn(std::shared_ptr<Connection> pConn);
    void stopAllConn();
private:
    ConnManager(){}

private:
    std::set<std::shared_ptr<Connection>> connList;

    static ConnManager* pInstance;
};


#endif //SERVERFRAME_CONNMANAGER_H