#pragma once
#include "../../Common/ErrorCode.h"
#include <mysql.h>
#include <unordered_map>
#include <iostream>
using ERROR_CODE = NCommon::ERROR_CODE;

namespace NLogicLib {

	//만들어야 하는 기능. 초기화한 conn
	class MysqlManager {

		
	public:
		MysqlManager();
		~MysqlManager();

		ERROR_CODE Init();
		ERROR_CODE Query(char* queryText);
		ERROR_CODE GetInfo();
		ERROR_CODE GetInfo(std::unordered_map<std::string, std::string>& bfUsers);
		void Disconnect();

	private:
		MYSQL* conn;
		MYSQL_RES* res;
		MYSQL_ROW row;
	};
}