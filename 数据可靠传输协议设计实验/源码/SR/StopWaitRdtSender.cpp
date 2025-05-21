#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"

SRRdtSender::SRRdtSender() : base(0), expectSequenceNumberSend(0), waitingState(false)
{
	for (int i = 0; i < 2 * Configuration::WINDOW_LEN; i++)
	{
		this->packetAcked[i] = false;
		this->packetWaitingAck[i].seqnum = -1;
	}
}

SRRdtSender::~SRRdtSender()
{
}

bool SRRdtSender::getWaitingState()
{
	return waitingState;
}

bool SRRdtSender::send(const Message &message)
{
	if (this->waitingState) // 窗口已满，无法发送任何数据
	{
		return false;
	}
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	const int send_num = expectSequenceNumberSend % seq_len; // 当前要发送的报文所在的窗口位置
	if (expectSequenceNumberSend < window_len + base) // 未超出窗口，可以发送
	{
		this->packetWaitingAck[send_num].acknum = -1; // 忽略该字段
		this->packetWaitingAck[send_num].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[send_num].checksum = 0;
		memcpy(this->packetWaitingAck[send_num].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[send_num].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[send_num]);
		this->packetAcked[send_num] = false;
		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[send_num]);
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[send_num].seqnum); // 启动发送方定时器
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[send_num]); // 发送置网络层						 // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		expectSequenceNumberSend++;											 // 已发送报文入队
		if (expectSequenceNumberSend == window_len + base)					 // 窗口已满，进入等待状态
		{
			this->waitingState = true;
		}
	}
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt)
{
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// 检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// 如果校验和正确，并且确认序号 >= 队头序号，作如下处理；否则什么都不做
	if (checkSum == ackPkt.checksum)
	{
		int ack_num = ackPkt.acknum % seq_len; // 当前获得确认的报文所在的窗口位置
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		if (this->base < ackPkt.acknum && !packetAcked[ack_num]) // 确认了一个失序报文
		{
			pns->stopTimer(SENDER, ackPkt.acknum);				  // 关闭定时器
			this->packetAcked[ack_num] = true;
		}
		else if (this->base == ackPkt.acknum) // 确认了一个按序报文
		{
			pns->stopTimer(SENDER, ackPkt.acknum);				  // 关闭定时器
			this->packetAcked[ack_num] = true;
			this->waitingState = false;
			// 滑动窗口前移
			while (this->packetAcked[this->base % seq_len])
			{
				this->packetAcked[this->base % seq_len] = false;
				this->packetWaitingAck[this->base % seq_len].seqnum = -1;
				this->base++;
			}
			// 打印滑动窗口
			for (int i = window_len + this->base; i < seq_len + this->base; i++)
			{
				this->packetWaitingAck[i % seq_len].seqnum = -1;
			}
			cout << "发送方滑动窗口移至: { ";
			for (int i = base; i < window_len + base; i++)
			{
				if (this->packetWaitingAck[i % seq_len].seqnum != -1)
				{
					cout << this->packetWaitingAck[i % seq_len].seqnum << ' ';
				}
				else
				{
					cout << '_' << ' ';
				}
			}
			cout << "}" << endl;
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum)
{
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	pns->stopTimer(SENDER, seqNum);							  // 首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); // 重新启动发送方定时器
	pUtils->printPacket("发送方定时器时间到，重发报文", this->packetWaitingAck[seqNum % seq_len]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % seq_len]); // 重新发送数据包
}
