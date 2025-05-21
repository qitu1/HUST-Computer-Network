#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
class SRRdtSender : public RdtSender
{
private:
	int base;												// ���Ͷ��еĶ�ͷ
	int expectSequenceNumberSend;							// ��һ���������
	bool waitingState;										// �Ƿ��ڵȴ�Ack��״̬
	bool packetAcked[2 * Configuration::WINDOW_LEN];		// ��Ǵ����ڵ�ÿ�������Ƿ��Ѿ���ȷ�ϡ�Ϊ�˷����ӡ�������ڣ����䳤������Ϊ���ڴ�С�� 2 ��
	Packet packetWaitingAck[2 * Configuration::WINDOW_LEN]; // �ѷ��Ͳ��ȴ�Ack�����ݰ���Ϊ�˷����ӡ�������ڣ����䳤������Ϊ���ڴ�С�� 2 ��

public:
	bool getWaitingState();
	bool send(const Message &message);	// ����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet &ackPkt); // ����ȷ��Ack������NetworkServiceSimulator����
	void timeoutHandler(int seqNum);	// Timeout handler������NetworkServiceSimulator����

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};

#endif
