#include "RankAndRdbmsUpdate.h"
#include <iostream>
#include <string.h>
#include <string>
#include <format>
namespace NLogicLib {

    RankAndRdbmsUpdate::RankAndRdbmsUpdate() {}
    RankAndRdbmsUpdate::~RankAndRdbmsUpdate() {}

    ERROR_CODE RankAndRdbmsUpdate::Init(RedisManager* pRedis, MysqlManager* pMysqlMgr, UserManager* pUserMgr, NServerNetLib::ITcpNetwork* pNetwork) {
        
        m_pRefRedisMgr = pRedis;
        m_pRefMysqlMgr = pMysqlMgr;
        m_pRefUserMgr = pUserMgr;
        m_pRefNetwork = pNetwork;
        isRunning = true;
        return ERROR_CODE::NONE;
    }

    void RankAndRdbmsUpdate::Run() {
        while (1) {
            if (isRunning) {
                //여기 구현.
                //RDBMS에 저장.
                std::unordered_map<std::string, User*> &users = m_pRefUserMgr->GetConnectedUsers();
                for (auto member : users) {
                    //member.first;
                    std::string userscnt = "";

                    std::cout << "redis요청.." << std::endl;
                    {

                        cpp_redis::client& client = m_pRefRedisMgr->GetRedisCli();
                        std::mutex& mtx = m_pRefRedisMgr->getmutex();

                        std::cout << member.first << std::endl;
                        std::lock_guard<std::mutex> lock(mtx);
                        client.get(std::string(member.first), [&userscnt, member](cpp_redis::reply& reply) {
                            if (reply.is_null()) {
                                std::cout << "유저가 채팅입력하지않음. 0번임" << std::endl;
                            }
                            else if (reply.is_integer()) {
                                //userscnt = reply.as_integer();
                                std::cout << "bulk_string으로 리턴되서 이럴일 없음" << std::endl;

                            }
                            else {

                                userscnt = reply.as_string();
                                std::cout <<member.first<<": "<<  reply << std::endl;
                                cpp_redis::reply::type type = reply.get_type();

                                switch (type) {
                                case cpp_redis::reply::type::integer:
                                    std::cout << "Reply type is integer" << std::endl;
                                    break;
                                case cpp_redis::reply::type::simple_string:
                                    std::cout << "Reply type is string" << std::endl;
                                    break;
                                case cpp_redis::reply::type::array:
                                    std::cout << "Reply type is array" << std::endl;
                                    break;
                                case cpp_redis::reply::type::bulk_string:
                                    std::cout << "Reply type is bulk string" << std::endl;
                                    break;
                                case cpp_redis::reply::type::error:
                                    std::cout << "Reply type is error" << std::endl;
                                    break;
                                case cpp_redis::reply::type::null:
                                    std::cout << "Reply type is nil" << std::endl;
                                    break;
                                default:
                                    std::cout << "Unknown reply type" << std::endl;
                                    break;
                                }
                            }


                            });
                        client.sync_commit();
                    }
                   
                    //redis에 요청
                    std::cout << "REBS로 이동.." << std::endl;
                    std::string query = "UPDATE chat_count SET count=" +userscnt+ " WHERE id ='" + std::string(member.first)+"'";
                    //해당 정보를 RDBMS에 넣기

                    std::cout << query << std::endl;
                    m_pRefMysqlMgr->Query(const_cast<char*>(query.c_str()));
                }
                
                //User목록을 들고있는 클래스 필요.
            }
            if (endcheck == true) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    void RankAndRdbmsUpdate::End() {
        isRunning = false;
        endcheck = true;
    }
    

}