#include <tuple>

#include "../../Common/Packet.h"
#include "../../Common/ErrorCode.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "User.h"
#include "UserManager.h"
#include "LobbyManager.h"
#include "Lobby.h"
#include "Game.h"
#include "Room.h"
#include "PacketProcess.h"
#include <iostream>
#include <locale>
#include <string>
#include <tchar.h>
#include <string_view>
#include <cuchar>
#include <codecvt>
#include <charconv>>
#include <atlstr.h>
#include <io.h>
#include <fcntl.h>
#include <mutex>
using PACKET_ID = NCommon::PACKET_ID;
using namespace std::string_literals;
namespace NLogicLib
{
	ERROR_CODE PacketProcess::RoomEnter(PacketInfo packetInfo)
	{
		std::cout << "ROOM ENTER 패킷받음" << std::endl;
		auto reqPkt = (NCommon::PktRoomEnterReq*)packetInfo.pRefData;

		NCommon::PktRoomEnterRes resPkt;

		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		if (errorCode != ERROR_CODE::NONE) {
			resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, sizeof(resPkt), (char*)&resPkt);
            return errorCode;
		}
				

		//로비상태에서만 룸에들어갈 수 있음 (이미 룸에있으면 안됨)
		if (pUser->IsCurDomainInLobby() == false) {
			resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_ENTER_INVALID_DOMAIN;
		}
		//로비 상태 무시 로그인 -> Room으로 구현


		auto lobbyIndex = pUser->GetLobbyIndex();
		auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
		if (pLobby == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_LOBBY_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_ENTER_INVALID_LOBBY_INDEX;
		}
		
		Room* pRoom = nullptr;
		

		//해당 Index의 룸에 들어간다.
		pRoom = pLobby->GetRoom(reqPkt->RoomIndex);
		if (pRoom == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX);
			m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, sizeof(resPkt), (char*)&resPkt);
			return ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX;
		}


		auto enterRet = pRoom->EnterUser(pUser);
		if (enterRet != ERROR_CODE::NONE) {
			resPkt.SetError(enterRet);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, sizeof(resPkt), (char*)&resPkt);
            return enterRet;
		}
		
		// 유저 정보를 룸에 들어왔다고 변경한다.
		pUser->EnterRoom(lobbyIndex, pRoom->GetIndex());		
		
		// 룸에 새 유저 들어왔다고 알린다
		//pRoom->NotifyEnterUserInfo(pUser->GetIndex(), pUser->GetID().c_str());
		pRoom->NotifyUserListInfo();
		//나는 방있음.
		//새로운 유저면 기존 유저 정보를 받아야한다.

		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, sizeof(resPkt), (char*)&resPkt);
		return ERROR_CODE::NONE;
	}

	ERROR_CODE PacketProcess::RoomLeave(PacketInfo packetInfo)
	{
		NCommon::PktRoomLeaveRes resPkt;

		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		if (errorCode != ERROR_CODE::NONE) {
			resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_LEAVE_RES, sizeof(resPkt), (char*)&resPkt);
            return errorCode;
		}

		auto userIndex = pUser->GetIndex();

		if (pUser->IsCurDomainInRoom() == false) {
			resPkt.SetError(ERROR_CODE::ROOM_LEAVE_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_LEAVE_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_LEAVE_INVALID_DOMAIN;
		}

		auto lobbyIndex = pUser->GetLobbyIndex();
		auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
		if (pLobby == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_LOBBY_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_LEAVE_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_ENTER_INVALID_LOBBY_INDEX;
		}

		auto pRoom = pLobby->GetRoom(pUser->GetRoomIndex());
		if (pRoom == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_LEAVE_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX;
		}

		auto leaveRet = pRoom->LeaveUser(userIndex);
		if (leaveRet != ERROR_CODE::NONE) {
			resPkt.SetError(leaveRet);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_LEAVE_RES, sizeof(resPkt), (char*)&resPkt);
            return leaveRet;
		}
		
		// 유저 정보를 로비로 변경
		pUser->EnterLobby(lobbyIndex);

		// 룸에 유저가 나갔음을 통보
		pRoom->NotifyLeaveUserInfo(pUser->GetID().c_str());
				
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_LEAVE_RES, sizeof(resPkt), (char*)&resPkt);
		return ERROR_CODE::NONE;
	}



	ERROR_CODE PacketProcess::RoomChat(PacketInfo packetInfo)
	{
		std::cout << "채팅 패킷 받음" << std::endl;
		
		auto reqPkt = (NCommon::PktRoomChatReq*)packetInfo.pRefData;
		NCommon::PktRoomChatRes resPkt;
		
		WCHAR unicode[256];
		char ansi[256];
		memset(unicode, 0, sizeof(unicode));
		memset(ansi, 0, sizeof(ansi));
		string temp  = string(reqPkt->Msg, reqPkt->Msg + reqPkt->len);
		::MultiByteToWideChar(CP_UTF8, 0, temp.c_str(), -1, unicode, reqPkt->len);
		::WideCharToMultiByte(CP_ACP, 0, unicode, -1, ansi, reqPkt->len, NULL, NULL);

		//std::string(ansi);
		std::cout << std::string(ansi) << std::endl;




		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		//Redis 작업
		//m_pRefRedisMgr->client.set("hello", "redis1");


		cpp_redis::client &redisCli = m_pRefRedisMgr->GetRedisCli();
		//ID, 메시지 보낸 횟수
		std::cout << pUser->GetID()<< ": ";

		{
			std::mutex& mtx = m_pRefRedisMgr->getmutex();
			std::lock_guard<std::mutex> lock(mtx);

			//TEST

			//ENDTEST

			redisCli.incr(pUser->GetID(), [](cpp_redis::reply& reply) {
				if (reply.is_integer()) {
					std::cout << reply.as_integer() << std::endl;
				}
				else {
					std::cout << "ERror" << std::endl;
				}
				});
			redisCli.commit();
		}
		
		
		if (errorCode != ERROR_CODE::NONE) {
			resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
            return errorCode;
		}

		if (pUser->IsCurDomainInRoom() == false) {
			resPkt.SetError(ERROR_CODE::ROOM_CHAT_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_CHAT_INVALID_DOMAIN;
		}

		auto lobbyIndex = pUser->GetLobbyIndex();
		auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
		if (pLobby == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_CHAT_INVALID_LOBBY_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_CHAT_INVALID_LOBBY_INDEX;
		}

		auto pRoom = pLobby->GetRoom(pUser->GetRoomIndex());
		if (pRoom == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX;
		}

		pRoom->NotifyChat(pUser->GetSessioIndex(), pUser->GetID().c_str(), reqPkt->Msg, reqPkt->len);
				
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
		return ERROR_CODE::NONE;
	}

	//TODO 아직 게임 로직은 다 구현하지 못했습니다.
	ERROR_CODE PacketProcess::RoomMasterGameStart(PacketInfo packetInfo)
	{
		NCommon::PktRoomMaterGameStartRes resPkt;

		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		if (errorCode != ERROR_CODE::NONE) {
			resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return errorCode;
		}

		if (pUser->IsCurDomainInRoom() == false) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_DOMAIN;
		}

		auto lobbyIndex = pUser->GetLobbyIndex();
		auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
		if (pLobby == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_LOBBY_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_LOBBY_INDEX;
		}

		auto pRoom = pLobby->GetRoom(pUser->GetRoomIndex());
		if (pRoom == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_ROOM_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_ROOM_INDEX;
		}

		// 방장이 맞는지 확인
		if (pRoom->IsMaster(pUser->GetIndex()) == false) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_MASTER);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_MASTER;
		}

		// 방의 인원이 2명인가?
		if (pRoom->GetUserCount() != 2) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_USER_COUNT);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_USER_COUNT;
		}

		// 방의 상태가 게임을 안하는 중인지?
		if (pRoom->GetGameObj()->GetState() != GameState::NONE) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_GAME_STATE);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_GAME_STATE;
		}

		// 방의 게임 상태 변경
		pRoom->GetGameObj()->SetState(GameState::STARTTING);
				
		// 방의 다른 유저에게 방장이 게임 시작 요청을 했음을 알리고
		pRoom->SendToAllUser((short)PACKET_ID::ROOM_MASTER_GAME_START_NTF, 
								0, 
								nullptr, 
								pUser->GetIndex());

		// 요청자에게 답변을 보낸다.
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_MASTER_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
		return ERROR_CODE::NONE;
	}

	ERROR_CODE PacketProcess::RoomGameStart(PacketInfo packetInfo)
	{
		NCommon::PktRoomGameStartRes resPkt;

		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		if (errorCode != ERROR_CODE::NONE) {
			resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return errorCode;
		}
				
		if (pUser->IsCurDomainInRoom() == false) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_DOMAIN;
		}

		auto lobbyIndex = pUser->GetLobbyIndex();
		auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
		if (pLobby == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_LOBBY_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_LOBBY_INDEX;
		}

		auto pRoom = pLobby->GetRoom(pUser->GetRoomIndex());
		if (pRoom == nullptr) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_ROOM_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_ROOM_INDEX;
		}

		// 방의 상태가 게임을 안하는 중인지?
		if (pRoom->GetGameObj()->GetState() != GameState::STARTTING) {
			resPkt.SetError(ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_GAME_STATE);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_MASTER_GAME_START_INVALID_GAME_STATE;
		}

		//TODO: 이미 게임 시작 요청을 했는가?

		//TODO: 방에서 게임 시작 요청한 유저 리스트에 등록

		// 방의 다른 유저에게 게임 시작 요청을 했음을 알리고

		// 요청자에게 답변을 보낸다.
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
		
		
		// 게임 시작 가능한가?
		// 시작이면 게임 상태 변경 GameState::ING
		// 게임 시작 패킷 보내기
		// 방의 상태 변경 로비에 알리고
		// 게임의 선택 시작 시간 설정
		return ERROR_CODE::NONE;
	}
}