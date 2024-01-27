#include "RedisManager.h"
#include <iostream>
#include <string.h>


namespace NLogicLib {

    RedisManager::RedisManager() {}
    RedisManager::~RedisManager() {}

    ERROR_CODE RedisManager::Init() {
        client.connect();

        client.set("hello", "redis");
        client.get("hello", [](cpp_redis::reply& reply) {
            std::cout << reply << std::endl;
            });
        
        client.sync_commit();

        return ERROR_CODE::NONE;
    }

    std::mutex& RedisManager::getmutex() {
        return mtx;
    }

    cpp_redis::client& RedisManager::GetRedisCli() {
        return client;
    }
    
}