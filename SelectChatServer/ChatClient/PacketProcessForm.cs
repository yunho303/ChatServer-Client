using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace csharp_test_client
{
    public partial class mainForm
    {
        Dictionary<PACKET_ID, Action<byte[]>> PacketFuncDic = new Dictionary<PACKET_ID, Action<byte[]>>();

        void SetPacketHandler()
        {
            //PacketFuncDic.Add(PACKET_ID.PACKET_ID_ERROR_NTF, PacketProcess_ErrorNotify);
            PacketFuncDic.Add(PACKET_ID.LOGIN_RES, PacketProcess_LoginResponse);
            PacketFuncDic.Add(PACKET_ID.ROOM_ENTER_RES, PacketProcess_RoomEnterResponse);
            PacketFuncDic.Add(PACKET_ID.ROOM_USER_LIST_NTF, PacketProcess_RoomUserListNotify);
            PacketFuncDic.Add(PACKET_ID.ROOM_NEW_USER_NTF, PacketProcess_RoomNewUserNotify);
            PacketFuncDic.Add(PACKET_ID.ROOM_LEAVE_RES, PacketProcess_RoomLeaveResponse);
            PacketFuncDic.Add(PACKET_ID.ROOM_LEAVE_USER_NTF, PacketProcess_RoomLeaveUserNotify);
            PacketFuncDic.Add(PACKET_ID.ROOM_CHAT_RES, PacketProcess_RoomChatResponse);            
            PacketFuncDic.Add(PACKET_ID.ROOM_CHAT_NOTIFY, PacketProcess_RoomChatNotify);
            //PacketFuncDic.Add(PACKET_ID.PACKET_ID_ROOM_RELAY_NTF, PacketProcess_RoomRelayNotify);
            PacketFuncDic.Add(PACKET_ID.DEV_ECHO_RES, PacketProcess_EchoRes);
            PacketFuncDic.Add(PACKET_ID.LOBBY_LIST_RES, PacketProcess_LobbyListResponse);
            PacketFuncDic.Add(PACKET_ID.LOBBY_ENTER_RES, PacketProcess_LobbyEnterResponse);
            
        }
        
        void PacketProcess(PacketData packet)
        {
            var packetType = (PACKET_ID)packet.PacketID;
            //DevLog.Write("Packet Error:  PacketID:{packet.PacketID.ToString()},  Error: {(ERROR_CODE)packet.Result}");
            //DevLog.Write("RawPacket: " + packet.PacketID.ToString() + ", " + PacketDump.Bytes(packet.BodyData));

            if (PacketFuncDic.ContainsKey(packetType))
            {
                PacketFuncDic[packetType](packet.BodyData);
            }
            else
            {
                DevLog.Write("Unknown Packet Id: " + packet.PacketID.ToString());
            }         
        }

        public struct PktLobbyEnterRes
        {
            public Int16 Result;
            public short MaxUserCount;
            public short MaxRoomCount;
        };
        void PacketProcess_LobbyEnterResponse(byte[] bodyData)
        {        
            var PktLobbyEnterRes = new PktLobbyEnterRes();

            PktLobbyEnterRes.Result = BitConverter.ToInt16(bodyData, 0);
            PktLobbyEnterRes.MaxUserCount = BitConverter.ToInt16(bodyData, 2);
            PktLobbyEnterRes.MaxRoomCount = BitConverter.ToInt16(bodyData, 4);
            if ((ERROR_CODE)PktLobbyEnterRes.Result != ERROR_CODE.ERROR_NONE)
            {
                DevLog.Write($"이미 로비 안에 있습니다.");
                DevLog.Write($"LobbyEnter Result: {(ERROR_CODE)PktLobbyEnterRes.Result}");
            }

            DevLog.Write($"LobbyEnter Result: {(ERROR_CODE)PktLobbyEnterRes.Result}");
            DevLog.Write($"0에서 {PktLobbyEnterRes.MaxRoomCount} 중 방 번호를 선택하세요");
        }
        void PacketProcess_LobbyListResponse(byte[] bodyData)
        {
            var PktLobbyListRes = new PktLobbyListRes();

            PktLobbyListRes.FromBytes(bodyData);

            DevLog.Write($"LobbyRequest Result: {(ERROR_CODE)PktLobbyListRes.Result}");
            listBoxLobby.Items.Clear();
            for (int i = 0; i < PktLobbyListRes.LobbyCount; i++)
            {
                string temp = "Lobby Id: ";
                temp += PktLobbyListRes.LobbyList[i].LobbyId.ToString();
                temp += "   UserCnt: ";
                temp += PktLobbyListRes.LobbyList[i].LobbyUserCount.ToString();
                temp += "/";
                temp += PktLobbyListRes.LobbyList[i].LobbyMaxUserCount.ToString();
                listBoxLobby.Items.Add(temp);
            }
            



        }

        void PacketProcess_EchoRes(byte[] bodyData)
        {
            var echoResponsePkt = new EchoResPacket();

            echoResponsePkt.FromBytes(bodyData);



            DevLog.Write($"ECHO: {echoResponsePkt.echoPkt}  ERROR: {echoResponsePkt.Result}", LOG_LEVEL.INFO);
        }
        void PacketProcess_ErrorNotify(byte[] bodyData)
        {
            var notifyPkt = new ErrorNtfPacket();
            notifyPkt.FromBytes(bodyData);

            DevLog.Write($"에러 통보 받음:  {notifyPkt.Error}");
        }


        void PacketProcess_LoginResponse(byte[] bodyData)
        {
            var responsePkt = new LoginResPacket();
            responsePkt.FromBytes(bodyData);

            DevLog.Write($"로그인 결과:  {(ERROR_CODE)responsePkt.Result}");
        }

        

        void PacketProcess_RoomEnterResponse(byte[] bodyData)
        {
            //DevLog.Write($"응답패킷");
            var responsePkt = new RoomEnterResPacket();
            responsePkt.Result = BitConverter.ToInt16(bodyData, 0);
            if((ERROR_CODE)responsePkt.Result != ERROR_CODE.ERROR_NONE) {
                DevLog.Write($"방 입장 문제 발생:  {(ERROR_CODE)responsePkt.Result}");
                return;
            }
            //responsePkt.FromBytes(bodyData);

            DevLog.Write($"방 입장 결과:  {(ERROR_CODE)responsePkt.Result}");
        }

        void PacketProcess_RoomUserListNotify(byte[] bodyData)
        {
            var notifyPkt = new RoomUserListNtfPacket();
            notifyPkt.FromBytes(bodyData);

            //그냥 다 받자.
            listBoxRoomUserList.Items.Clear();
            for (int i = 0; i < notifyPkt.UserCount; ++i)
            {
                AddRoomUserList(notifyPkt.UserIDList[i]);
            }

            DevLog.Write($"방의 기존 유저 리스트 받음");
        }

        void PacketProcess_RoomNewUserNotify(byte[] bodyData)
        {
            DevLog.Write($"새로운 유저 들어왔음");
            var notifyPkt = new RoomNewUserNtfPacket();
            notifyPkt.FromBytes(bodyData);

            AddRoomUserList(notifyPkt.UserID);
            
            DevLog.Write($"방에 새로 들어온 유저 받음");
        }


        void PacketProcess_RoomLeaveResponse(byte[] bodyData)
        {
            var responsePkt = new RoomLeaveResPacket();
            responsePkt.FromBytes(bodyData);

            DevLog.Write($"방 나가기 결과:  {(ERROR_CODE)responsePkt.Result}");
        }

        void PacketProcess_RoomLeaveUserNotify(byte[] bodyData)
        {
            var notifyPkt = new RoomLeaveUserNtfPacket();
            notifyPkt.FromBytes(bodyData);

            RemoveRoomUserList(notifyPkt.UserUniqueId);

            DevLog.Write($"방에서 나간 유저 받음");
        }


        void PacketProcess_RoomChatResponse(byte[] bodyData)
        {
            var responsePkt = new RoomChatResPacket();
            responsePkt.FromBytes(bodyData);

            var errorCode = (ERROR_CODE)responsePkt.Result;
            var msg = $"방 채팅 요청 결과:  {(ERROR_CODE)responsePkt.Result}";
            if (errorCode == ERROR_CODE.ERROR_NONE)
            {
                DevLog.Write(msg, LOG_LEVEL.ERROR);
            }/*
            else
            {
                AddRoomChatMessageList(0, msg);
            }*/
        }


        void PacketProcess_RoomChatNotify(byte[] bodyData)
        {
            var responsePkt = new RoomChatNtfPacket();
            responsePkt.FromBytes(bodyData);

            AddRoomChatMessageList(responsePkt.UserUniqueId, responsePkt.Message);
        }
        
        void AddRoomChatMessageList(string userUniqueId, string msgssage)
        {
            var msg = $"{userUniqueId}:  {msgssage}";

            if (listBoxRoomChatMsg.Items.Count > 512)
            {
                listBoxRoomChatMsg.Items.Clear();
            }

            listBoxRoomChatMsg.Items.Add(msg);
            listBoxRoomChatMsg.SelectedIndex = listBoxRoomChatMsg.Items.Count - 1;
        }


        void PacketProcess_RoomRelayNotify(byte[] bodyData)
        {
            var notifyPkt = new RoomRelayNtfPacket();
            notifyPkt.FromBytes(bodyData);

            var stringData = Encoding.UTF8.GetString(notifyPkt.RelayData);
            DevLog.Write($"방에서 릴레이 받음. {notifyPkt.UserUniqueId} - {stringData}");
        }
    }
}
