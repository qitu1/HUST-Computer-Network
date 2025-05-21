#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class SRRdtReceiver : public RdtReceiver
{
private:
	int base;
	int expectSequenceNumberRcvd;						// 期待收到的下一个报文序号
	Packet lastAckPkt;									// 上次发送的确认报文
	bool packetReceived[2 * Configuration::WINDOW_LEN]; // 报文是否收到
	Packet cachedPkt[2 * Configuration::WINDOW_LEN];	// 已收到但未传到应用层的报文

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	void receive(const Packet &packet); // 接收报文，将被NetworkService调用
};

#endif
