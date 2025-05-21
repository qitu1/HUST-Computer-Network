#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"

EasyTCPRdtSender::EasyTCPRdtSender() : base(0), expectSequenceNumberSend(0), newest_ack({-1, 0}), waitingState(false)
{
	for (int i = 0; i < 2 * Configuration::WINDOW_LEN; i++)
	{
		this->packetWaitingAck[i].seqnum = -1;
	}
}

EasyTCPRdtSender::~EasyTCPRdtSender()
{
}

bool EasyTCPRdtSender::getWaitingState()
{
	return waitingState;
}

bool EasyTCPRdtSender::send(const Message &message)
{
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	const int send_num = expectSequenceNumberSend % seq_len; // 当前要发送的报文所在的窗口位置
	if (this->waitingState) // 窗口已满，无法发送任何数据
	{
		return false;
	}
	if (expectSequenceNumberSend < window_len + this->base) // 未超出窗口，可以发送
	{
		this->packetWaitingAck[send_num].acknum = -1; // 忽略该字段
		this->packetWaitingAck[send_num].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[send_num].checksum = 0;
		memcpy(this->packetWaitingAck[send_num].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[send_num].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[send_num]);
		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[send_num]);
		if (expectSequenceNumberSend == this->base) // 发送前窗口为空
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[send_num].seqnum); // 启动发送方定时器
		}
		
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[send_num]); // 发送置网络层						 // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		expectSequenceNumberSend++;											 // 已发送报文入队
		if (expectSequenceNumberSend == window_len + base)					 // 窗口已满，进入等待状态
		{
			this->waitingState = true;
		}
	}
	return true;
}

void EasyTCPRdtSender::receive(const Packet &ackPkt)
{
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// 检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// 如果校验和正确，并且确认序号 >= 队头序号，作如下处理；否则什么都不做
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= this->base)
	{
		int ack_num = ackPkt.acknum % seq_len; // 当前获得确认的报文所在的窗口位置
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		pns->stopTimer(SENDER, this->base);				  // 关闭定时器
		if (expectSequenceNumberSend != 1 + ackPkt.acknum) // 已发送的报文未全部确认
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, 1 + ackPkt.acknum);				  // 启动定时器
		}
		this->base = 1 + ackPkt.acknum;
		this->waitingState = false;
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
	else if (checkSum == ackPkt.checksum)
	// 如果收到了重复的 ACK
	{
		if (this->newest_ack.first == ackPkt.acknum)
		// 处理快速重传
		{
			this->newest_ack.second++;
			cout << "发送方收到冗余ACK " << ackPkt.acknum << endl;
			if (this->newest_ack.second == 3)
			{
				cout << "快速重传序号是 " << 1 + ackPkt.acknum << " 的报文" << endl;
				pns->stopTimer(SENDER, 1 + ackPkt.acknum);
				pns->startTimer(SENDER, Configuration::TIME_OUT, 1 + ackPkt.acknum);
				pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[(1 + ackPkt.acknum) % seq_len]);
				this->newest_ack.second -= 3;
			}
		}
		else
		{
			this->newest_ack = {ackPkt.acknum, 0};
		}
	}
}

void EasyTCPRdtSender::timeoutHandler(int seqNum)
{
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	pns->stopTimer(SENDER, seqNum);							  // 首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); // 重新启动发送方定时器
	pUtils->printPacket("发送方定时器时间到，重发报文", this->packetWaitingAck[seqNum % seq_len]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % seq_len]); // 重新发送数据包
}
