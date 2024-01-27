#pragma once
#include "../../Common/ErrorCode.h"
#include "../ServerNetLib/ServerNetErrorCode.h"
#include "../ServerNetLib/Define.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "UserManager.h"
#include "RedisManager.h"
#include "MysqlManager.h"
#include <cpp_redis>
#include <mysql.h>
#include <iostream>
using ERROR_CODE = NCommon::ERROR_CODE;


namespace NServerNetLib
{
	struct ServerConfig;
	class ILog;
	class ITcpNetwork;
}

namespace NLogicLib {
	class RedisManager;
	class MysqlManager;
	class UserManager;


	//만들어야 하는 기능. 초기화한 conn
	class RankAndRdbmsUpdate {


	public:
		RankAndRdbmsUpdate();
		~RankAndRdbmsUpdate();

		ERROR_CODE Init(RedisManager* pRedis, MysqlManager* pMysqlMgr, UserManager* pUserMgr, 
			NServerNetLib::ITcpNetwork* pNetwork);
		void Run();
		void End();
		


	private:
		RedisManager* m_pRefRedisMgr;
		MysqlManager* m_pRefMysqlMgr;
		UserManager * m_pRefUserMgr;
		NServerNetLib::ITcpNetwork* m_pRefNetwork;
		bool isRunning = false;
		bool endcheck = false;

		//std::unique_ptr<PacketProcess> m_pPacketProc;
	};
}
