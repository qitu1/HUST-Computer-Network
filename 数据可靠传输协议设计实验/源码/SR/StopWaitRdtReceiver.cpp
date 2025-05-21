#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtReceiver.h"

SRRdtReceiver::SRRdtReceiver() : base(0), expectSequenceNumberRcvd(0)
{
	lastAckPkt.acknum = -1; // 初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1; // 忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
	{
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < 2 * Configuration::WINDOW_LEN; i++)
	{
		this->packetReceived[i] = false;
		this->cachedPkt[i].acknum = -1;
		this->cachedPkt[i].seqnum = -1;
	}
}

SRRdtReceiver::~SRRdtReceiver()
{
}

void SRRdtReceiver::receive(const Packet &packet)
{
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// 检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	// 打印接收方窗口
	cout << "接收方接收前窗口: { ";
	for (int i = this->base; i < window_len + this->base; i++)
	{
		if (this->packetReceived[i % seq_len])
		{
			cout << i << ' ';
		}
		else
		{
			cout << '_' << ' ';
		}
	}
	cout << "}" << endl;
	// 如果校验和正确
	if (checkSum == packet.checksum)
	{
		if (packet.seqnum < window_len + base && base < packet.seqnum) // 收到失序报文
		{
			pUtils->printPacket("接收方正确收到发送方的报文", packet);
			this->cachedPkt[packet.seqnum % seq_len] = packet;
			this->packetReceived[packet.seqnum % seq_len] = true;
			// 发送 ACK
			lastAckPkt.acknum = packet.seqnum; // 确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt); // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		}
		else if (base == packet.seqnum) // 收到按序报文
		{
			pUtils->printPacket("接收方正确收到发送方的报文", packet);
			this->cachedPkt[packet.seqnum % seq_len] = packet;
			this->packetReceived[packet.seqnum % seq_len] = true;
			this->cachedPkt[packet.seqnum % seq_len].acknum = 0;
			// 发送 ACK
			lastAckPkt.acknum = packet.seqnum; // 确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt); // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
			// 滑动窗口
			while (this->packetReceived[this->base % seq_len] == true)
			{
				// 取出Message，向上递交给应用层
				Message msg;
				memcpy(msg.data, cachedPkt[this->base % seq_len].payload, sizeof(cachedPkt[this->base % seq_len].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				this->packetReceived[this->base % seq_len] = false;
				this->cachedPkt[packet.seqnum % seq_len].acknum = -1;
				this->base++;
			}
		}
		else if (packet.seqnum < base && packet.seqnum >= base - window_len) //收到已经确认的报文
		{
			pUtils->printPacket("接收方收到重复报文", packet);
			lastAckPkt.acknum = packet.seqnum; // 确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt); // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		}
		else
		{
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			cout << "接收方的滑动窗口：" << base << " & " << window_len + base << endl;
		}
	}
	else
	{
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
	}
}