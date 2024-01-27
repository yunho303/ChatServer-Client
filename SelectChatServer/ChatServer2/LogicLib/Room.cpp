#include <algorithm>
#include <cstring>
#include <wchar.h>

#include "../ServerNetLib/ILog.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "../../Common/Packet.h"
#include "../../Common/ErrorCode.h"

#include "User.h"
#include "Game.h"
#include "Room.h"
#include <iostream>
using PACKET_ID = NCommon::PACKET_ID;

namespace NLogicLib
{
	Room::Room() {}

	Room::~Room()
	{
		if (m_pGame != nullptr) {
			delete m_pGame;
		}
	}
	
	void Room::Init(const short index, const short maxUserCount)
	{
		m_Index = index;
		m_MaxUserCount = maxUserCount;

		m_pGame = new Game;
	}

	void Room::SetNetwork(TcpNet* pNetwork, ILog* pLogger)
	{
		m_pRefLogger = pLogger;
		m_pRefNetwork = pNetwork;
	}

	void Room::Clear()
	{
		m_IsUsed = false;
		m_Title = L"";
		m_UserList.clear();
	}

	Game* Room::GetGameObj()
	{
		return m_pGame;
	}

	ERROR_CODE Room::CreateRoom(const wchar_t* pRoomTitle)
	{
		if (m_IsUsed) {
			return ERROR_CODE::ROOM_ENTER_CREATE_FAIL;
		}

		m_IsUsed = true;
		m_Title = pRoomTitle;

		return ERROR_CODE::NONE;
	}

	ERROR_CODE Room::EnterUser(User* pUser)
	{
		/*if (m_IsUsed == false) {
			return ERROR_CODE::ROOM_ENTER_NOT_CREATED;
		}*/

		if (m_UserList.size() == m_MaxUserCount) {
			return ERROR_CODE::ROOM_ENTER_MEMBER_FULL;
		}

		m_UserList.push_back(pUser);
		return ERROR_CODE::NONE;
	}

	bool Room::IsMaster(const short userIndex)
	{
		return m_UserList[0]->GetIndex() == userIndex ? true : false;
	}

	ERROR_CODE Room::LeaveUser(const short userIndex)
	{
		if (m_IsUsed == false) {
			return ERROR_CODE::ROOM_ENTER_NOT_CREATED;
		}

		auto iter = std::find_if(std::begin(m_UserList), std::end(m_UserList), [userIndex](auto pUser) { return pUser->GetIndex() == userIndex; });
		if (iter == std::end(m_UserList)) {
			return ERROR_CODE::ROOM_LEAVE_NOT_MEMBER;
		}
		
		m_UserList.erase(iter);

		if (m_UserList.empty()) 
		{
			Clear();
		}

		return ERROR_CODE::NONE;
	}

	void Room::SendToAllUser(const short packetId, const short dataSize, char* pData, const int passUserindex)
	{
		for (auto pUser : m_UserList)
		{
			if (pUser->GetIndex() == passUserindex) {
				continue;
			}

			m_pRefNetwork->SendData(pUser->GetSessioIndex(), packetId, dataSize, pData);
		}
	}

	void Room::NotifyEnterUserInfo(const int userIndex, const char* pszUserID)
	{
		NCommon::PktRoomEnterUserInfoNtf pkt;
#ifdef _WIN32
		strncpy_s(pkt.UserID, (NCommon::MAX_USER_ID_SIZE+1), pszUserID, NCommon::MAX_USER_ID_SIZE);
#else
        std::strncpy(pkt.UserID, pszUserID, NCommon::MAX_USER_ID_SIZE);
#endif

		SendToAllUser((short)PACKET_ID::ROOM_ENTER_NEW_USER_NTF, sizeof(pkt), (char*)&pkt, userIndex);
	}


	void Room::NotifyUserListInfo()
	{
		//룸 리스트에있는 친구들 정보를 준다.
		NCommon::RoomUserListNtfPacket pkt;
		pkt.Usernum = (short)m_UserList.size();
		int index = 0;

		
		for (int i = 0; i < m_UserList.size(); i++) {
			string tempstring = m_UserList[i]->GetID();
			std::cout << tempstring << std::endl;
			int tempsize = tempstring.size();
			std::memcpy(pkt.Users + index, (void*)&tempsize, sizeof(int));
			index += 4;
			std::memcpy(pkt.Users + index, tempstring.c_str(), tempstring.size());
			index += tempstring.size();
			//std::cout << tempstring.size() << " " << sizeof(tempstring) << std::endl;
			
		}
		/*int temp;
		std::cout << "1번유저" << std::endl;
		std::memcpy(&temp, pkt.Users, sizeof(int));
		std::cout << temp << std::endl;

		char name[20];
		std::memcpy(&name, pkt.Users+ 4, temp);
		name[temp] = '\0';
		std::cout << name << std::endl;*/


		//std::cout << sizeof(pkt) << std::endl;
		SendToAllUser((short)PACKET_ID::ROOM_USER_LIST_NTF, sizeof(pkt), (char*)&pkt,-1);

	}



	void Room::NotifyLeaveUserInfo(const char* pszUserID)
	{
		if (m_IsUsed == false) {
			return;
		}

		NCommon::PktRoomLeaveUserInfoNtf pkt;
#ifdef _WIN32
		strncpy_s(pkt.UserID, (NCommon::MAX_USER_ID_SIZE+1), pszUserID, NCommon::MAX_USER_ID_SIZE);
#else
        std::strncpy(pkt.UserID, pszUserID, NCommon::MAX_USER_ID_SIZE);
#endif
		SendToAllUser((short)PACKET_ID::ROOM_LEAVE_USER_NTF, sizeof(pkt), (char*)&pkt);
	}

	void Room::NotifyChat(const int sessionIndex, const char* pszUserID, const char* pszMsg, short pktLen)
	{
		NCommon::PktRoomChatNtf pkt;
		//std::cout << sizeof(pkt) << std::endl;
#ifdef _WIN32
		pkt.IDSize = string(pszUserID).length();
		//std::cout << pkt.IDSize << std::endl;
		strncpy_s(pkt.UserID, (NCommon::MAX_USER_ID_SIZE+1), pszUserID, NCommon::MAX_USER_ID_SIZE);
		pkt.MsgSize = string(pszMsg).length();
		//std::cout << pkt.MsgSize << std::endl;
		strncpy_s(pkt.Msg, NCommon::MAX_ROOM_CHAT_MSG_SIZE + 1, pszMsg, pktLen);
#else
        std::strncpy(pkt.UserID, pszUserID, NCommon::MAX_USER_ID_SIZE);
        std::wcsncpy(pkt.Msg, pszMsg, NCommon::MAX_ROOM_CHAT_MSG_SIZE);
#endif
		SendToAllUser((short)PACKET_ID::ROOM_CHAT_NTF, sizeof(pkt), (char*)&pkt, -1);
	}

	void Room::Update()
	{
		if (m_pGame->GetState() == GameState::ING)
		{
			if (m_pGame->CheckSelectTime())
			{
				//선택 안하는 사람이 지도록 한
			}
		}
	}
}