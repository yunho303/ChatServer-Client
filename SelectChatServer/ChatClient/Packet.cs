
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Net.Sockets;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;



namespace csharp_test_client
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    struct PacketData
    {
        public Int16 DataSize;
        public Int16 PacketID;
        public SByte Type;
        public byte[] BodyData;
    }

    /*public class RcvPktBase
    {
        short ErrorCode = (short)ERROR_CODE.ERROR_NONE;
    }*/
    public class PacketDump
    {
        public static string Bytes(byte[] byteArr)
        {
            StringBuilder sb = new StringBuilder("[");
            for (int i = 0; i < byteArr.Length; ++i)
            {
                sb.Append(byteArr[i] + " ");
            }
            sb.Append("]");
            return sb.ToString();
        }
    }
    
    public struct LobbyListInfo
    {
        public Int16 LobbyId;
        public Int16 LobbyUserCount;
        public Int16 LobbyMaxUserCount;
    };
    public class PktLobbyListRes
    {
        public Int16 Result;
        public Int16 LobbyCount = 0;
        public LobbyListInfo[] LobbyList = new LobbyListInfo[20];

        public bool FromBytes(byte[] bodyData)
        {
            int index = 0;
            Result = BitConverter.ToInt16(bodyData, index);
            index += 2;
            LobbyCount = BitConverter.ToInt16(bodyData, index);
            index += 2;
            
            for (int i = 0; i < LobbyCount; i++)
            {
                //DevLog.Write("LobbyCount" + LobbyCount);
                //DevLog.Write("Indx" +index);
                LobbyList[i].LobbyId = BitConverter.ToInt16(bodyData, index);
                index += 2;
                LobbyList[i].LobbyUserCount = BitConverter.ToInt16(bodyData, index);
                index += 2;
                LobbyList[i].LobbyMaxUserCount = BitConverter.ToInt16(bodyData, index);
                index += 2;
            }

            return true;
        }
    }

    public class EchoResPacket
    {
        public Int16 Result;
        public string echoPkt;

        public bool FromBytes(byte[] bodyData)
        {
            Result = BitConverter.ToInt16(bodyData, 0);
            byte[] subArray = new byte[bodyData.Length - 2];
            Array.Copy(bodyData, 2, subArray, 0, bodyData.Length - 2);
            echoPkt = Encoding.UTF8.GetString(subArray);
            return true;
        }
    }


    public class ErrorNtfPacket
    {
        public ERROR_CODE Error;

        public bool FromBytes(byte[] bodyData)
        {
            Error = (ERROR_CODE)BitConverter.ToInt16(bodyData, 0);
            return true;
        }
    }
    

    public class LoginReqPacket
    {
        byte[] UserID = new byte[PacketDef.MAX_USER_ID_BYTE_LENGTH];
        byte[] UserPW = new byte[PacketDef.MAX_USER_PW_BYTE_LENGTH];

        public void SetValue(string userID, string userPW)
        {
            Encoding.UTF8.GetBytes(userID).CopyTo(UserID, 0);
            Encoding.UTF8.GetBytes(userPW).CopyTo(UserPW, 0);
        }

        public byte[] ToBytes()
        {
            List<byte> dataSource = new List<byte>();
            dataSource.AddRange(UserID);
            dataSource.AddRange(UserPW);
            return dataSource.ToArray();
        }
    }

    public class LoginResPacket
    {
        public Int16 Result;

        public bool FromBytes(byte[] bodyData)
        {
            Result = BitConverter.ToInt16(bodyData, 0);
            return true;
        }
    }


    public class RoomEnterReqPacket
    {
        int RoomNumber;
        public void SetValue(int roomNumber)
        {
            RoomNumber = roomNumber;
        }

        public byte[] ToBytes()
        {
            List<byte> dataSource = new List<byte>();
            dataSource.AddRange(BitConverter.GetBytes(RoomNumber));
            return dataSource.ToArray();
        }
    }

    public class RoomEnterResPacket
    {
        public Int16 Result;
        public Int64 RoomUserUniqueId;

        public bool FromBytes(byte[] bodyData)
        {
            Result = BitConverter.ToInt16(bodyData, 0);
            RoomUserUniqueId = BitConverter.ToInt64(bodyData, 2);
            return true;
        }
    }

    public class RoomUserListNtfPacket
    {
        public Int16 UserCount = 0;
        public List<string> UserIDList = new List<string>();

        public bool FromBytes(byte[] bodyData)
        {
            var readPos = 0;
            UserCount = BitConverter.ToInt16( bodyData , 0);
            readPos += 2;
            DevLog.Write("UserCount" + UserCount);
            for (int i = 0; i < UserCount; i++)
            {
                int stringlength = BitConverter.ToInt32(bodyData , readPos);
                readPos += 4;
                DevLog.Write("stringlength" + stringlength);
                byte[] subArray = new byte[stringlength+1];
                Array.Copy(bodyData, readPos, subArray, 0, stringlength);
                
                DevLog.Write(Encoding.UTF8.GetString(subArray));
                UserIDList.Add(Encoding.UTF8.GetString(subArray));
                readPos += stringlength;
            }

            return true;
        }
    }

    public class RoomNewUserNtfPacket
    {
        //public Int64 UserUniqueId;
        public string UserID;

        public bool FromBytes(byte[] bodyData)
        {
            
            UserID = Encoding.UTF8.GetString(bodyData);

            return true;
        }
    }


    public class RoomChatReqPacket
    {
        public Int16 MsgLen;
        byte[] Msg;//= new byte[PacketDef.MAX_USER_ID_BYTE_LENGTH];

        public void SetValue(string message)
        {
            
            Msg = Encoding.UTF8.GetBytes(message);
            DevLog.Write($"{Msg}");
            MsgLen = (Int16)Msg.Length;
        }

        public byte[] ToBytes()
        {
            List<byte> dataSource = new List<byte>();
            dataSource.AddRange(BitConverter.GetBytes(MsgLen));
            dataSource.AddRange(Msg);
            //DevLog.Write($"{dataSource.Count}");
            return dataSource.ToArray();

        }
    }

    public class RoomChatResPacket
    {
        public Int16 Result;
        
        public bool FromBytes(byte[] bodyData)
        {
            Result = BitConverter.ToInt16(bodyData, 0);
            return true;
        }
    }

    public class RoomChatNtfPacket
    {
        public short IdLen;
        public string UserUniqueId;
        public short  MessageLen;
        public string Message;

        public bool FromBytes(byte[] bodyData)
        {
            int index = 0;
            IdLen = BitConverter.ToInt16((byte[])bodyData, 0);
            index += 2;
            byte[] IdTemp = new byte[IdLen];
            Buffer.BlockCopy(bodyData, index, IdTemp, 0, IdLen);   
            UserUniqueId = Encoding.UTF8.GetString(IdTemp);
            //DevLog.Write($"{UserUniqueId.Length}");     
            index += 17;
            MessageLen = BitConverter.ToInt16((byte[])bodyData, index);
            index += 2;
            byte[] MsgTemp = new byte[MessageLen];
            
            Buffer.BlockCopy(bodyData, index, MsgTemp, 0, MessageLen);

            Message = Encoding.UTF8.GetString(MsgTemp);
            //DevLog.Write($"{Message.Length}");
            return true;
        }
    }


     public class RoomLeaveResPacket
    {
        public Int16 Result;
        
        public bool FromBytes(byte[] bodyData)
        {
            Result = BitConverter.ToInt16(bodyData, 0);
            return true;
        }
    }

    public class RoomLeaveUserNtfPacket
    {
        public Int64 UserUniqueId;

        public bool FromBytes(byte[] bodyData)
        {
            UserUniqueId = BitConverter.ToInt64(bodyData, 0);
            return true;
        }
    }


    
    public class RoomRelayNtfPacket
    {
        public Int64 UserUniqueId;
        public byte[] RelayData;

        public bool FromBytes(byte[] bodyData)
        {
            UserUniqueId = BitConverter.ToInt64(bodyData, 0);

            var relayDataLen = bodyData.Length - 8;
            RelayData = new byte[relayDataLen];
            Buffer.BlockCopy(bodyData, 8, RelayData, 0, relayDataLen);
            return true;
        }
    }


    public class PingRequest
    {
        public Int16 PingNum;

        public byte[] ToBytes()
        {
            return BitConverter.GetBytes(PingNum);
        }

    }
}
