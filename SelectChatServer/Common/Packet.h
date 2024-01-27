#pragma once
#include <vector>
#include "PacketID.h"
#include "ErrorCode.h"
#include <cuchar>
#include <string>

namespace NCommon
{	
#pragma pack(push, 1)
	struct PktHeader
	{
		short TotalSize;
		short Id;
		unsigned char Reserve;
	};

	struct PktBase
	{
		short ErrorCode = (short)ERROR_CODE::NONE;
		void SetError(ERROR_CODE error) { ErrorCode = (short)error; }
	};

	//- 로그인 요청
	const int MAX_USER_ID_SIZE = 16;
	const int MAX_USER_PASSWORD_SIZE = 16;
	struct PktLogInReq
	{
		char szID[MAX_USER_ID_SIZE] = { 0, };
		char szPW[MAX_USER_PASSWORD_SIZE] = { 0, };
	};

	struct PktLogInRes : PktBase
	{
	};


	//- 채널 리스트 요청
	// Request에는 Body가 없음.

	const int MAX_LOBBY_LIST_COUNT = 20;
	struct LobbyListInfo
	{
		short LobbyId;
		short LobbyUserCount;
		short LobbyMaxUserCount;
	};
	struct PktLobbyListRes : PktBase
	{
		short LobbyCount = 0;
		LobbyListInfo LobbyList[MAX_LOBBY_LIST_COUNT];
	};


	//- 로비 입장 요청
	struct PktLobbyEnterReq
	{
		short LobbyId;
	};
		
	struct PktLobbyEnterRes : PktBase
	{
		short MaxUserCount;
		short MaxRoomCount;
	};


	
	//- 로비에서 나가기 요청
	struct PktLobbyLeaveReq {};

	struct PktLobbyLeaveRes : PktBase
	{
	};
	
	

	//- 룸에 들어가기 요청
	const int MAX_ROOM_TITLE_SIZE = 16;
	struct PktRoomEnterReq
	{
		int RoomIndex;
	};

	struct PktRoomEnterRes : PktBase
	{
	};

		
	//- 룸에 있는 유저에게 새로 들어온 유저 정보 통보
	struct PktRoomEnterUserInfoNtf
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};

	struct RoomUserListNtfPacket
	{
		//어떻게 작성?
		short Usernum;
		char Users[1024];
	};


	//- 룸 나가기 요청
	struct PktRoomLeaveReq {};

	struct PktRoomLeaveRes : PktBase
	{
	};

	//- 룸에서 나가는 유저 통보(로비에 있는 유저에게)
	struct PktRoomLeaveUserInfoNtf
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};
		

	//- 룸 채팅
	const int MAX_ROOM_CHAT_MSG_SIZE = 256;
	struct PktRoomChatReq
	{
		short len;
		char Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};

	struct PktRoomChatRes : PktBase
	{
	};

	struct PktRoomChatNtf
	{
		short IDSize;
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
		short MsgSize;
		char Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};

	//- 로비 채팅
	const int MAX_LOBBY_CHAT_MSG_SIZE = 256;
	struct PktLobbyChatReq
	{
		wchar_t Msg[MAX_LOBBY_CHAT_MSG_SIZE + 1] = { 0, };
	};

	struct PktLobbyChatRes : PktBase
	{
	};

	struct PktLobbyChatNtf
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
		wchar_t Msg[MAX_LOBBY_CHAT_MSG_SIZE + 1] = { 0, };
	};


	// 방장의 게임 시작 요청
	struct PktRoomMaterGameStartReq
	{};

	struct PktRoomMaterGameStartRes : PktBase
	{};

	struct PktRoomMaterGameStartNtf
	{};


	// 방장이 아닌 사람의 게임 시작 요청
	struct PktRoomGameStartReq
	{};

	struct PktRoomGameStartRes : PktBase
	{};

	struct PktRoomGameStartNtf
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};




	const int DEV_ECHO_DATA_MAX_SIZE = 1024;

	struct PktDevEchoReq
	{
		//short DataSize;
		char Datas[DEV_ECHO_DATA_MAX_SIZE];
	};

	struct PktDevEchoRes : PktBase
	{
		//short DataSize;
		char Datas[DEV_ECHO_DATA_MAX_SIZE];
	};

#pragma pack(pop)


	
}