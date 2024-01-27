#include "MysqlManager.h"
#include <iostream>
#include <string.h>


namespace NLogicLib {

	MysqlManager::MysqlManager() {}
	MysqlManager::~MysqlManager() {}

    ERROR_CODE MysqlManager::Init() {

        //conn을 통해 쿼리를 수행한다.
		conn = mysql_init(NULL);
        if (conn == NULL) {
            fprintf(stderr, "mysql_init() failed\n");
            return ERROR_CODE::MYSQL_SETTING_ERROR;
        }

        // MySQL에 연결
        if (mysql_real_connect(conn, "localhost", "root", "비밀번호시지", "userinfo", 3306, NULL, 0) == NULL) {
            fprintf(stderr, "mysql_real_connect() failed\n");
            mysql_close(conn);
            return ERROR_CODE::MYSQL_SETTING_ERROR;
        }

        return ERROR_CODE::NONE;
	}

    ERROR_CODE MysqlManager::Query(char* queryText) {
        if (mysql_query(conn, queryText) != 0) {
            fprintf(stderr, "mysql_query() failed\n");
            //mysql_close(conn);
            return ERROR_CODE::MYSQL_QUARY_ERROR;
        }
        return ERROR_CODE::NONE;
    }
    ERROR_CODE  MysqlManager::GetInfo()
    {
        res = mysql_store_result(conn);
        if (res == NULL) {
            fprintf(stderr, "mysql_store_result() failed\n");
            //mysql_close(conn);
            return ERROR_CODE::MYSQL_NO_RESULT;
        }

        // 결과 출력
        while ((row = mysql_fetch_row(res)) != NULL) {
            printf("Id: %s ", row[0]);
            printf("Password: %s\n", row[1]);
            // 필요한 만큼 계속 출력
        }
        return ERROR_CODE::NONE;
    }
    ERROR_CODE  MysqlManager::GetInfo(std::unordered_map<std::string, std::string>& bfUsers)
    {
        res = mysql_store_result(conn);
        if (res == NULL) {
            fprintf(stderr, "mysql_store_result() failed\n");
            //mysql_close(conn);
            return ERROR_CODE::MYSQL_NO_RESULT;
        }

        // 결과 출력
        while ((row = mysql_fetch_row(res)) != NULL) {
            printf("Id: %s ", row[0]);
            printf("Password 2: %s\n", row[1]);
            bfUsers.insert({row[0], row[1]});
            // 필요한 만큼 계속 출력
        }
        return ERROR_CODE::NONE;
    }

    void MysqlManager::Disconnect() {
        mysql_free_result(res);
        mysql_close(conn);
    }
}