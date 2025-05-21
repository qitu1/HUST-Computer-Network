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
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	const int send_num = expectSequenceNumberSend % seq_len; // ��ǰҪ���͵ı������ڵĴ���λ��
	if (this->waitingState) // �����������޷������κ�����
	{
		return false;
	}
	if (expectSequenceNumberSend < window_len + this->base) // δ�������ڣ����Է���
	{
		this->packetWaitingAck[send_num].acknum = -1; // ���Ը��ֶ�
		this->packetWaitingAck[send_num].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[send_num].checksum = 0;
		memcpy(this->packetWaitingAck[send_num].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[send_num].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[send_num]);
		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[send_num]);
		if (expectSequenceNumberSend == this->base) // ����ǰ����Ϊ��
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[send_num].seqnum); // �������ͷ���ʱ��
		}
		
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[send_num]); // �����������						 // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
		expectSequenceNumberSend++;											 // �ѷ��ͱ������
		if (expectSequenceNumberSend == window_len + base)					 // ��������������ȴ�״̬
		{
			this->waitingState = true;
		}
	}
	return true;
}

void EasyTCPRdtSender::receive(const Packet &ackPkt)
{
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// ���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// ���У�����ȷ������ȷ����� >= ��ͷ��ţ������´�������ʲô������
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= this->base)
	{
		int ack_num = ackPkt.acknum % seq_len; // ��ǰ���ȷ�ϵı������ڵĴ���λ��
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		pns->stopTimer(SENDER, this->base);				  // �رն�ʱ��
		if (expectSequenceNumberSend != 1 + ackPkt.acknum) // �ѷ��͵ı���δȫ��ȷ��
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, 1 + ackPkt.acknum);				  // ������ʱ��
		}
		this->base = 1 + ackPkt.acknum;
		this->waitingState = false;
		// ��ӡ��������
		for (int i = window_len + this->base; i < seq_len + this->base; i++)
		{
			this->packetWaitingAck[i % seq_len].seqnum = -1;
		}
		cout << "���ͷ�������������: { ";
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
	// ����յ����ظ��� ACK
	{
		if (this->newest_ack.first == ackPkt.acknum)
		// ��������ش�
		{
			this->newest_ack.second++;
			cout << "���ͷ��յ�����ACK " << ackPkt.acknum << endl;
			if (this->newest_ack.second == 3)
			{
				cout << "�����ش������ " << 1 + ackPkt.acknum << " �ı���" << endl;
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
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	pns->stopTimer(SENDER, seqNum);							  // ���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); // �����������ͷ���ʱ��
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�����", this->packetWaitingAck[seqNum % seq_len]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % seq_len]); // ���·������ݰ�
}
