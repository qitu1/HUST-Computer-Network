#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
class SRRdtSender : public RdtSender
{
private:
	int base;												// 发送队列的队头
	int expectSequenceNumberSend;							// 下一个发送序号
	bool waitingState;										// 是否处于等待Ack的状态
	bool packetAcked[2 * Configuration::WINDOW_LEN];		// 标记窗口内的每个报文是否已经被确认。为了方便打印滑动窗口，将其长度设置为窗口大小的 2 倍
	Packet packetWaitingAck[2 * Configuration::WINDOW_LEN]; // 已发送并等待Ack的数据包。为了方便打印滑动窗口，将其长度设置为窗口大小的 2 倍

public:
	bool getWaitingState();
	bool send(const Message &message);	// 发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt); // 接受确认Ack，将被NetworkServiceSimulator调用
	void timeoutHandler(int seqNum);	// Timeout handler，将被NetworkServiceSimulator调用

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};

#endif
