using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace csharp_test_client
{

    public partial class mainForm : Form
    {
        ClientSimpleTcp Network = new ClientSimpleTcp();

        bool IsNetworkThreadRunning = false;
        bool IsBackGroundProcessRunning = false;

        System.Threading.Thread NetworkReadThread = null;
        System.Threading.Thread NetworkSendThread = null;

        PacketBufferManager PacketBuffer = new PacketBufferManager();
        Queue<PacketData> RecvPacketQueue = new Queue<PacketData>();
        Queue<byte[]> SendPacketQueue = new Queue<byte[]>();

        Timer dispatcherUITimer;


        public mainForm()
        {
            InitializeComponent();
        }

        private void mainForm_Load(object sender, EventArgs e)
        {
            PacketBuffer.Init((8096 * 10), PacketDef.PACKET_HEADER_SIZE, 1024);

            IsNetworkThreadRunning = true;
            NetworkReadThread = new System.Threading.Thread(this.NetworkReadProcess);
            NetworkReadThread.Start();
            NetworkSendThread = new System.Threading.Thread(this.NetworkSendProcess);
            NetworkSendThread.Start();

            IsBackGroundProcessRunning = true;
            dispatcherUITimer = new Timer();
            dispatcherUITimer.Tick += new EventHandler(BackGroundProcess);
            dispatcherUITimer.Interval = 100;
            dispatcherUITimer.Start();

            btnDisconnect.Enabled = false;

            SetPacketHandler();
            DevLog.Write("프로그램 시작 !!!", LOG_LEVEL.INFO);
        }

        private void mainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            IsNetworkThreadRunning = false;
            IsBackGroundProcessRunning = false;

            Network.Close();
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            string address = textBoxIP.Text;

            if (checkBoxLocalHostIP.Checked)
            {
                address = "127.0.0.1";
            }

            int port = Convert.ToInt32(textBoxPort.Text);

            if (Network.Connect(address, port))
            {
                labelStatus.Text = string.Format("{0}. 서버에 접속 중", DateTime.Now);
                btnConnect.Enabled = false;
                btnDisconnect.Enabled = true;

                DevLog.Write($"서버에 접속 중", LOG_LEVEL.INFO);
            }
            else
            {
                labelStatus.Text = string.Format("{0}. 서버에 접속 실패", DateTime.Now);
            }
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            SetDisconnectd();
            Network.Close();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(textSendText.Text))
            {
                MessageBox.Show("보낼 텍스트를 입력하세요");
                return;
            }

            //if(Network)

            /*public Int16 DataSize;
            public Int16 PacketID;
            public SByte Type;
            public byte[] BodyData;*/
            List<byte> dataSource = new List<byte>();

            PacketData packetData = new PacketData();

            // 5 + dataSource.Count, 241
            packetData.DataSize = (Int16)Encoding.UTF8.GetBytes(textSendText.Text).Length;
            packetData.DataSize += PacketDef.PACKET_HEADER_SIZE;
            dataSource.AddRange(BitConverter.GetBytes(packetData.DataSize));
            //DevLog.Write($"{dataSource.Count}", LOG_LEVEL.INFO);
            packetData.PacketID = (Int16)PACKET_ID.DEV_ECHO_REQ;
            dataSource.AddRange(BitConverter.GetBytes(packetData.PacketID));
            //DevLog.Write($"{dataSource.Count}", LOG_LEVEL.INFO);

            dataSource.AddRange(new byte[] { (byte)0 });

            //DevLog.Write($"{dataSource.Count}", LOG_LEVEL.INFO);
            dataSource.AddRange(Encoding.UTF8.GetBytes(textSendText.Text));
            /*byte[] sourceText = Encoding.UTF8.GetBytes(textSendText.Text);
            packetData.BodyData = new byte[sourceText.Length];
            Buffer.BlockCopy(sourceText, 0, packetData.BodyData, 0, sourceText.Length);*/

            //DevLog.Write($"{dataSource.Count}", LOG_LEVEL.INFO);

            SendPacketQueue.Enqueue(dataSource.ToArray());
        }



        void NetworkReadProcess()
        {
            const Int16 PacketHeaderSize = PacketDef.PACKET_HEADER_SIZE;

            while (IsNetworkThreadRunning)
            {
                if (Network.IsConnected() == false)
                {
                    System.Threading.Thread.Sleep(1);
                    continue;
                }

                var recvData = Network.Receive();

                if (recvData != null)
                {
                    PacketBuffer.Write(recvData.Item2, 0, recvData.Item1); // 2가 바이트 배열, 1이 길이
                    //버퍼에 쓴다.
                    while (true)
                    {
                        var data = PacketBuffer.Read();
                        if (data.Count < 1)
                        {
                            break;
                        }

                        var packet = new PacketData();
                        packet.DataSize = (short)(data.Count - PacketHeaderSize);
                        packet.PacketID = BitConverter.ToInt16(data.Array, data.Offset + 2);
                        packet.Type = (SByte)data.Array[(data.Offset + 4)];
                        packet.BodyData = new byte[packet.DataSize];
                        Buffer.BlockCopy(data.Array, (data.Offset + PacketHeaderSize), packet.BodyData, 0, (data.Count - PacketHeaderSize));
                        //DevLog.Write($"{packet.PacketID} {packet.DataSize}", LOG_LEVEL.INFO);



                        lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                        {
                            RecvPacketQueue.Enqueue(packet);
                        }
                    }
                    //DevLog.Write($"받은 데이터:{recvData.Item1}  {recvData.Item2}", LOG_LEVEL.INFO);
                }
                else
                {
                    Network.Close();
                    SetDisconnectd();
                    DevLog.Write("서버와 접속 종료 !!!", LOG_LEVEL.INFO);
                }
            }
        }

        void NetworkSendProcess()
        {
            while (IsNetworkThreadRunning)
            {
                System.Threading.Thread.Sleep(1);

                if (Network.IsConnected() == false)
                {
                    continue;
                }

                lock (((System.Collections.ICollection)SendPacketQueue).SyncRoot)
                {
                    if (SendPacketQueue.Count > 0)
                    {
                        var packet = SendPacketQueue.Dequeue();
                        Network.Send(packet);
                    }
                }
            }
        }


        void BackGroundProcess(object sender, EventArgs e)
        {
            ProcessLog();

            try
            {
                var packet = new PacketData();

                lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                {
                    if (RecvPacketQueue.Count() > 0)
                    {
                        packet = RecvPacketQueue.Dequeue();
                    }
                }

                if (packet.PacketID != 0)
                {
                    PacketProcess(packet);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("ReadPacketQueueProcess. error:{0}", ex.Message));
            }
        }

        private void ProcessLog()
        {
            // 너무 이 작업만 할 수 없으므로 일정 작업 이상을 하면 일단 패스한다.
            int logWorkCount = 0;

            while (IsBackGroundProcessRunning)
            {
                System.Threading.Thread.Sleep(1);

                string msg;

                if (DevLog.GetLog(out msg))
                {
                    ++logWorkCount;

                    if (listBoxLog.Items.Count > 512)
                    {
                        listBoxLog.Items.Clear();
                    }

                    listBoxLog.Items.Add(msg);
                    listBoxLog.SelectedIndex = listBoxLog.Items.Count - 1;
                }
                else
                {
                    break;
                }

                if (logWorkCount > 8)
                {
                    break;
                }
            }
        }


        public void SetDisconnectd()
        {
            if (btnConnect.InvokeRequired)
            {
                btnConnect.Invoke(new Action(() => { SetDisconnectd(); }));
            }
            else
            {
                if (btnConnect.Enabled == false)
                {
                    btnConnect.Enabled = true;
                    btnDisconnect.Enabled = false;
                }

                SendPacketQueue.Clear();

                listBoxRoomChatMsg.Items.Clear();
                listBoxRoomUserList.Items.Clear();

                labelStatus.Text = "서버 접속이 끊어짐";
            }

        }

        public void PostSendPacket(PACKET_ID packetID, byte[] bodyData)
        {
            if (Network.IsConnected() == false)
            {
                DevLog.Write("서버 연결이 되어 있지 않습니다", LOG_LEVEL.ERROR);
                return;
            }

            Int16 bodyDataSize = 0;
            if (bodyData != null)
            {
                bodyDataSize = (Int16)bodyData.Length;
            }
            var packetSize = bodyDataSize + PacketDef.PACKET_HEADER_SIZE;

            List<byte> dataSource = new List<byte>();
            dataSource.AddRange(BitConverter.GetBytes((Int16)packetSize));
            dataSource.AddRange(BitConverter.GetBytes((Int16)packetID));
            dataSource.AddRange(new byte[] { (byte)0 });

            if (bodyData != null)
            {
                dataSource.AddRange(bodyData);
            }

            SendPacketQueue.Enqueue(dataSource.ToArray());
        }

        void AddRoomUserList(string userID)
        {
            
            var msg = $"{userID}";
            listBoxRoomUserList.Items.Add(msg);
        }

        void RemoveRoomUserList(Int64 userUniqueId)
        {
            object removeItem = null;

            foreach (var user in listBoxRoomUserList.Items)
            {
                var items = user.ToString().Split(":");
                if (items[0].ToInt64() == userUniqueId)
                {
                    removeItem = user;
                    return;
                }
            }

            if (removeItem != null)
            {
                listBoxRoomUserList.Items.Remove(removeItem);
            }
        }


        // 로그인 요청
        private void button2_Click(object sender, EventArgs e)
        {
            var loginReq = new LoginReqPacket();
            loginReq.SetValue(textBoxUserID.Text, textBoxUserPW.Text);

            PostSendPacket(PACKET_ID.LOGIN_REQ, loginReq.ToBytes());
            DevLog.Write($"로그인 요청:  {textBoxUserID.Text}, {textBoxUserPW.Text}");
        }

        private void btn_RoomEnter_Click(object sender, EventArgs e)
        {
            var requestPkt = new RoomEnterReqPacket();
            requestPkt.SetValue(textBoxRoomNumber.Text.ToInt32());

            PostSendPacket(PACKET_ID.ROOM_ENTER_REQ, requestPkt.ToBytes());
            DevLog.Write($"방 입장 요청:  {textBoxRoomNumber.Text} 번");
        }

        private void btn_RoomLeave_Click(object sender, EventArgs e)
        {
            PostSendPacket(PACKET_ID.ROOM_LEAVE_REQ, null);
            DevLog.Write($"방 입장 요청:  {textBoxRoomNumber.Text} 번");
        }

        private void btnRoomChat_Click(object sender, EventArgs e)
        {
            if (textBoxRoomSendMsg.Text.IsEmpty())
            {
                MessageBox.Show("채팅 메시지를 입력하세요");
                return;
            }

            var requestPkt = new RoomChatReqPacket();
            requestPkt.SetValue(textBoxRoomSendMsg.Text);

            //DevLog.Write($"{requestPkt.MsgLen}");

            PostSendPacket(PACKET_ID.ROOM_CHAT_REQ, requestPkt.ToBytes());
            DevLog.Write($"방 채팅 요청");
        }

        private void btnRoomRelay_Click(object sender, EventArgs e)
        {
            //if( textBoxRelay.Text.IsEmpty())
            //{
            //    MessageBox.Show("릴레이 할 데이터가 없습니다");
            //    return;
            //}

            //var bodyData = Encoding.UTF8.GetBytes(textBoxRelay.Text);
            //PostSendPacket(PACKET_ID.PACKET_ID_ROOM_RELAY_REQ, bodyData);
            //DevLog.Write($"방 릴레이 요청");
        }

        // 로비 리스트 요청
        private void button3_Click(object sender, EventArgs e)
        {
            PostSendPacket(PACKET_ID.LOBBY_LIST_REQ, null);
            DevLog.Write($"방 릴레이 요청");
        }

        // 로비 입장 요청
        private void button4_Click(object sender, EventArgs e)
        {
            int selectedIndex = listBoxLobby.SelectedIndex;
            //선택된 인덱스.
            //DevLog.Write($"{selectedIndex}");
            if (selectedIndex == -1)
            {
                DevLog.Write($"Lobby를 선택해주세요");
                return;
            }
            byte[] byteArray = BitConverter.GetBytes((short)selectedIndex);
            PostSendPacket(PACKET_ID.LOBBY_ENTER_REQ, byteArray);

        }

        // 로비 나가기 요청
        private void button5_Click(object sender, EventArgs e)
        {

        }

        private void textBoxPort_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBoxIP_TextChanged(object sender, EventArgs e)
        {

        }

        private void checkBoxLocalHostIP_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void listBoxRoomUserList_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void listBoxRoomChatMsg_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void listBoxLobby_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void listBoxLog_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
