#pragma once
#include "../../Common/ErrorCode.h"
#include <cpp_redis>
#include <mysql.h>
#include <unordered_map>
#include <iostream>
#include <mutex>
using ERROR_CODE = NCommon::ERROR_CODE;

namespace NLogicLib {

	//������ �ϴ� ���. �ʱ�ȭ�� conn
	class RedisManager {


	public:
		RedisManager();
		~RedisManager();

		



		ERROR_CODE Init();
		cpp_redis::client& GetRedisCli();
		std::mutex& getmutex();

	private:
		cpp_redis::client client;
		std::mutex mtx;
	};
}
