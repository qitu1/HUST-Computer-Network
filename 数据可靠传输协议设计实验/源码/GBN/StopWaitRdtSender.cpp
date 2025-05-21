#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"

GBNRdtSender::GBNRdtSender() : base(0), expectSequenceNumberSend(0), waitingState(false)
{
	for (int i = 0; i < 2 * Configuration::WINDOW_LEN; i++)
	{
		packetWaitingAck[i].seqnum = -1;
	}
}

GBNRdtSender::~GBNRdtSender()
{
}

bool GBNRdtSender::getWaitingState()
{
	return waitingState;
}

bool GBNRdtSender::send(const Message &message)
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
		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[send_num]);
	}
	if (base == expectSequenceNumberSend) // 目前待确认队列为空，设置定时器
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[send_num].seqnum); // 启动发送方定时器
	}
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[send_num]); // 发送置网络层						 // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	expectSequenceNumberSend++;											 // 已发送报文入队
	if (expectSequenceNumberSend == window_len + base)					 // 窗口已满，进入等待状态
	{
		this->waitingState = true;
	}
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// 检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// 如果校验和正确，并且确认序号 >= 队头序号，作如下处理；否则什么都不做
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= this->base)
	{
		this->waitingState = false;
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		pns->stopTimer(SENDER, this->base);				  // 关闭定时器
		if (expectSequenceNumberSend > 1 + ackPkt.acknum) // 还有未确认的报文
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, 1 + ackPkt.acknum);//重新启动定时器
		}
		this->base = 1 + ackPkt.acknum; // 窗口滑动
		// 打印滑动窗口
		for (int i = window_len + this->base; i < seq_len + this->base; i++)
		{
			this->packetWaitingAck[i % seq_len].seqnum = -1;
		}
		cout << "滑动窗口移至: { ";
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

void GBNRdtSender::timeoutHandler(int seqNum)
{
	// 唯一一个定时器,无需考虑seqNum
	const int &window_len = Configuration::WINDOW_LEN; // 窗口大小简写
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	pns->stopTimer(SENDER, seqNum);							  // 首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); // 重新启动发送方定时器
	for (int i = this->base; i < this->expectSequenceNumberSend; i++)
	{
		pUtils->printPacket("发送方定时器时间到，重发报文", this->packetWaitingAck[i % seq_len]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[i % seq_len]); // 重新发送数据包
	}
}
