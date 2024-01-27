
#include <algorithm>

#include "../../Common/ErrorCode.h"
#include "MysqlManager.h"
#include "User.h" //여기서 namespace std; 가져옴.
#include "UserManager.h"

#include <iostream>


namespace NLogicLib
{
	UserManager::UserManager()
	{
	}

	UserManager::~UserManager()
	{
	}

	void UserManager::Init(const int maxUserCount , MysqlManager* pMysqlMgr)
	{
		for (int i = 0; i < maxUserCount; ++i)
		{
			User user;
			user.Init((short)i);

			m_UserObjPool.push_back(user);
			m_UserObjPoolIndex.push_back(i);
		}
		m_pMysqlMgr = pMysqlMgr;
		//유저 풀에 추가. maxUserCount 수 만큼.
		m_pMysqlMgr->Query("SELECT * FROM password");
		m_pMysqlMgr->GetInfo(m_BeforeUsers);
		//m_UserIDDic에 기존 유저 추가.
		//std::cout << m_BeforeUsers[0] << std::endl;

		
	}

	std::unordered_map<std::string, User*>& UserManager::GetConnectedUsers() {
		return m_UserIDDic;
	}

	User* UserManager::AllocUserObjPoolIndex()
	{
		if (m_UserObjPoolIndex.empty()) {
			return nullptr;
		}

		int index = m_UserObjPoolIndex.front();
		m_UserObjPoolIndex.pop_front();
		return &m_UserObjPool[index];
	}

	void UserManager::ReleaseUserObjPoolIndex(const int index)
	{
		m_UserObjPoolIndex.push_back(index);
		m_UserObjPool[index].Clear();
	}

	ERROR_CODE UserManager::AddUser(const int sessionIndex, const char* pszID, const char* pszPW)
	{
		//pszID-> 기존에 회원가입 or 새로
		//sessionIndex -> 지금 로그인. 시도 해당 함수는 지금 로그인 시도.

		//그렇다면. UsersessionDic, UserIDDic

		//새분화 해야한다. 데이터베이스에 있었던 유저인 경우부터.
		
		//만약 이미 로그인 되어있는 경우. sessionIndex가 부여



		auto foundDbUser = FindUserInDB(pszID);
		if (foundDbUser == "NOTFOUND") {
			//없는 유저임. 신규유저
			//처음 로그인한 유저인 경우한자리 때준다. 그냥 하나 더 만들자.
			auto pUser = AllocUserObjPoolIndex();
			if (pUser == nullptr) {
				return ERROR_CODE::USER_MGR_MAX_USER_COUNT;
			}

			pUser->Set(sessionIndex, pszID);
			pUser->setPassword(string(pszPW));
			m_UserSessionDic.insert({ sessionIndex, pUser });
			m_UserIDDic.insert({ pszID, pUser });

			string forquery = "INSERT INTO password VALUES('" + string(pszID) + "','" + string(pszPW) + "')";
			//DB에도 넣자.
			m_pMysqlMgr->Query(const_cast<char*>(forquery.c_str()));
			m_BeforeUsers.insert({ pszID ,pszPW});

			std::string query = "INSERT INTO chat_count VALUES('"+string(pszID) + "',0)";
			//해당 정보를 RDBMS에 넣기std::
			m_pMysqlMgr->Query(const_cast<char*>(query.c_str()));


			std::cout << "신규유저 가입" << std::endl;
			return ERROR_CODE::NONE;
		}
		else {
			//기존 유저.
			if (foundDbUser == string(pszPW)) {
				//로그인 성공.
				//로그인 여부.
				auto foundUser = FindUser(pszID);
				if (foundUser != nullptr) {
					return ERROR_CODE::USER_MGR_ALREADY_LOGIN;
				}
				else {
					//로그인이 되어있지않음.
					//만약 여기서 Session 이 연결되있으면 해제.
					auto findsession = FindUser(sessionIndex);
					if (findsession != nullptr) {
						//이미 연결되어있는 세션

						RemoveUser(sessionIndex);
					}

					auto pUser = AllocUserObjPoolIndex();


					if (pUser == nullptr) {
						return ERROR_CODE::USER_MGR_MAX_USER_COUNT;
					}

					pUser->Set(sessionIndex, pszID);
					pUser->setPassword(string(pszPW));
					m_UserSessionDic.insert({ sessionIndex, pUser });
					m_UserIDDic.insert({ pszID, pUser });

					return ERROR_CODE::NONE;
				}
			}
			else {
				//패스워드 실패
				std::cout << "비밀번호 오류" << std::endl;
				return ERROR_CODE::USER_MGR_WRONG_PASSWORD;
			}
		}
		return ERROR_CODE::NONE;
	}

	ERROR_CODE UserManager::RemoveUser(const int sessionIndex)
	{
		auto pUser = FindUser(sessionIndex);

		if (pUser == nullptr) {
			return ERROR_CODE::USER_MGR_REMOVE_INVALID_SESSION;
		}

		auto index = pUser->GetIndex();
		auto pszID = pUser->GetID();

		m_UserSessionDic.erase(sessionIndex);
		m_UserIDDic.erase(pszID.c_str());
		ReleaseUserObjPoolIndex(index);

		return ERROR_CODE::NONE;
	}

	std::tuple<ERROR_CODE, User*> UserManager::GetUser(const int sessionIndex)
	{
		auto pUser = FindUser(sessionIndex);

		if (pUser == nullptr) {
			return { ERROR_CODE::USER_MGR_INVALID_SESSION_INDEX, nullptr };
		}

		if (pUser->IsConfirm() == false) {
			return{ ERROR_CODE::USER_MGR_NOT_CONFIRM_USER, nullptr };
		}

		return{ ERROR_CODE::NONE, pUser };
	}

	User* UserManager::FindUser(const int sessionIndex)
	{
		auto findIter = m_UserSessionDic.find(sessionIndex);
		
		if (findIter == m_UserSessionDic.end()) {
			return nullptr;
		}
		
		//auto pUser = (User*)&findIter->second;
		return (User*)findIter->second;
	}

	User* UserManager::FindUser(const char* pszID)
	{
		auto findIter = m_UserIDDic.find(pszID);
		//반환하는 것은 pUser
		/*cout << m_UserIDDic.size() << endl;
		for (auto k : m_UserIDDic) {
			cout << k.first << endl;
		}*/
		if (findIter == m_UserIDDic.end()) {
			//cout << "?" << endl;
			return nullptr;
		}

		/*int size = 0;
		while (pszID[size] != '\0') {
			size++;
		}
		cout << findIter->first << size << endl;*/
		return (User*)findIter->second;
	}

	std::string UserManager::FindUserInDB(const char* pszID)
	{
		auto findIter = m_BeforeUsers.find(pszID);
		
		if (findIter == m_BeforeUsers.end()) {
			//cout << "?" << endl;
			return "NOTFOUND";
		}

		return m_BeforeUsers[pszID];
	}
}