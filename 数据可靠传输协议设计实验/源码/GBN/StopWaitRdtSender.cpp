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
	if (this->waitingState) // �����������޷������κ�����
	{
		return false;
	}
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	const int send_num = expectSequenceNumberSend % seq_len; // ��ǰҪ���͵ı������ڵĴ���λ��
	if (expectSequenceNumberSend < window_len + base) // δ�������ڣ����Է���
	{
		this->packetWaitingAck[send_num].acknum = -1; // ���Ը��ֶ�
		this->packetWaitingAck[send_num].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[send_num].checksum = 0;
		memcpy(this->packetWaitingAck[send_num].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[send_num].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[send_num]);
		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[send_num]);
	}
	if (base == expectSequenceNumberSend) // Ŀǰ��ȷ�϶���Ϊ�գ����ö�ʱ��
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[send_num].seqnum); // �������ͷ���ʱ��
	}
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[send_num]); // �����������						 // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	expectSequenceNumberSend++;											 // �ѷ��ͱ������
	if (expectSequenceNumberSend == window_len + base)					 // ��������������ȴ�״̬
	{
		this->waitingState = true;
	}
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// ���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// ���У�����ȷ������ȷ����� >= ��ͷ��ţ������´�������ʲô������
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= this->base)
	{
		this->waitingState = false;
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		pns->stopTimer(SENDER, this->base);				  // �رն�ʱ��
		if (expectSequenceNumberSend > 1 + ackPkt.acknum) // ����δȷ�ϵı���
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, 1 + ackPkt.acknum);//����������ʱ��
		}
		this->base = 1 + ackPkt.acknum; // ���ڻ���
		// ��ӡ��������
		for (int i = window_len + this->base; i < seq_len + this->base; i++)
		{
			this->packetWaitingAck[i % seq_len].seqnum = -1;
		}
		cout << "������������: { ";
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
	// Ψһһ����ʱ��,���迼��seqNum
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	pns->stopTimer(SENDER, seqNum);							  // ���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); // �����������ͷ���ʱ��
	for (int i = this->base; i < this->expectSequenceNumberSend; i++)
	{
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�����", this->packetWaitingAck[i % seq_len]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[i % seq_len]); // ���·������ݰ�
	}
}
