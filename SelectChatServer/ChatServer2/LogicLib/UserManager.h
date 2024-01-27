#pragma once

#include <unordered_map>
#include <deque>
#include <string>
#include <vector>

namespace NCommon
{
	enum class ERROR_CODE :short;
}
using ERROR_CODE = NCommon::ERROR_CODE;

namespace NLogicLib
{	
	class User;
	class MysqlManager;

	class UserManager
	{
	public:
		UserManager();
		virtual ~UserManager();

		void Init(const int maxUserCount, MysqlManager* pMysqlMgr);

		ERROR_CODE AddUser(const int sessionIndex, const char* pszID, const char* pszPW);
		ERROR_CODE RemoveUser(const int sessionIndex);

		std::tuple<ERROR_CODE,User*> GetUser(const int sessionIndex);
		std::unordered_map<std::string, User*>& GetConnectedUsers();
				
	private:
		User* AllocUserObjPoolIndex();
		void ReleaseUserObjPoolIndex(const int index);

		User* FindUser(const int sessionIndex);
		User* FindUser(const char* pszID);
		std::string FindUserInDB(const char* pszID);

	private:
		MysqlManager* m_pMysqlMgr;
		std::vector<User> m_UserObjPool;
		std::deque<int> m_UserObjPoolIndex;

		std::unordered_map<int, User*> m_UserSessionDic;
		std::unordered_map<std::string, User*> m_UserIDDic; //char*�� key�� ������
		std::unordered_map<std::string, std::string> m_BeforeUsers; //char*�� key�� ������
	};
}