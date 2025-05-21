#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class SRRdtReceiver : public RdtReceiver
{
private:
	int base;
	int expectSequenceNumberRcvd;						// �ڴ��յ�����һ���������
	Packet lastAckPkt;									// �ϴη��͵�ȷ�ϱ���
	bool packetReceived[2 * Configuration::WINDOW_LEN]; // �����Ƿ��յ�
	Packet cachedPkt[2 * Configuration::WINDOW_LEN];	// ���յ���δ����Ӧ�ò�ı���

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	void receive(const Packet &packet); // ���ձ��ģ�����NetworkService����
};

#endif
