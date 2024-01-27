﻿#include <tuple>

#include "../../Common/Packet.h"
#include "../../Common/ErrorCode.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "ConnectedUserManager.h"
#include "User.h"
#include "UserManager.h"
#include "LobbyManager.h"
#include "PacketProcess.h"
#include <iostream>
#include <tacopie/tacopie>
#include <cpp_redis>
#include <mysql.h>
using std::cout;
using std::endl;
using PACKET_ID = NCommon::PACKET_ID;

namespace NLogicLib
{
	ERROR_CODE PacketProcess::Login(PacketInfo packetInfo)
	{
		//TODO: 받은 데이터가 PktLogInReq 크기만큼인지 조사해야 한다.
		// 패스워드는 무조건 pass 해준다.
		// ID 중복이라면 에러 처리한다.

		NCommon::PktLogInRes resPkt;
		//NCommon::PktLogInReq* reqPkt = new NCommon::PktLogInReq();
		auto reqPkt = (NCommon::PktLogInReq*)packetInfo.pRefData;
		cout <<"입력받은 아이디 / 비밀번호: " << reqPkt->szID << " " << reqPkt->szPW << endl;
		
		
		auto addRet = m_pRefUserMgr->AddUser(packetInfo.SessionIndex, reqPkt->szID, reqPkt->szPW);

		if (addRet != ERROR_CODE::NONE) {
			//문제 없어도 새로 가입된 아이디라면 데이터베이스에 추가.
			resPkt.SetError(addRet);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOGIN_IN_RES, sizeof(NCommon::PktLogInRes), (char*)&resPkt);
            return addRet;
		}

		//비교 어디서? 기존아이디 , 새로 가입 아이디, 불일치 아이디.

		m_pConnectedUserManager->SetLogin(packetInfo.SessionIndex);

		resPkt.ErrorCode = (short)addRet;
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOGIN_IN_RES, sizeof(NCommon::PktLogInRes), (char*)&resPkt);

		return ERROR_CODE::NONE;
	}

	ERROR_CODE PacketProcess::LobbyList(PacketInfo packetInfo)
	{
		// 인증 받은 유저인가?
		// 아직 로비에 들어가지 않은 유저인가?
        NCommon::PktLobbyListRes resPkt;
		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		if (errorCode != ERROR_CODE::NONE) {
			resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LIST_RES, sizeof(resPkt), (char*)&resPkt);
            return errorCode;
		}
	
		if (pUser->IsCurDomainInLogIn() == false) {
			resPkt.SetError(ERROR_CODE::LOBBY_LIST_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LIST_RES, sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::LOBBY_LIST_INVALID_DOMAIN;
		}
		
		m_pRefLobbyMgr->SendLobbyListInfo(packetInfo.SessionIndex);
		
		return ERROR_CODE::NONE;
	}
}