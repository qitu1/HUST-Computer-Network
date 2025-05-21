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
		this->packetAcked[send_num] = false;
		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[send_num]);
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[send_num].seqnum); // �������ͷ���ʱ��
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[send_num]); // �����������						 // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
		expectSequenceNumberSend++;											 // �ѷ��ͱ������
		if (expectSequenceNumberSend == window_len + base)					 // ��������������ȴ�״̬
		{
			this->waitingState = true;
		}
	}
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt)
{
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// ���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// ���У�����ȷ������ȷ����� >= ��ͷ��ţ������´�������ʲô������
	if (checkSum == ackPkt.checksum)
	{
		int ack_num = ackPkt.acknum % seq_len; // ��ǰ���ȷ�ϵı������ڵĴ���λ��
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		if (this->base < ackPkt.acknum && !packetAcked[ack_num]) // ȷ����һ��ʧ����
		{
			pns->stopTimer(SENDER, ackPkt.acknum);				  // �رն�ʱ��
			this->packetAcked[ack_num] = true;
		}
		else if (this->base == ackPkt.acknum) // ȷ����һ��������
		{
			pns->stopTimer(SENDER, ackPkt.acknum);				  // �رն�ʱ��
			this->packetAcked[ack_num] = true;
			this->waitingState = false;
			// ��������ǰ��
			while (this->packetAcked[this->base % seq_len])
			{
				this->packetAcked[this->base % seq_len] = false;
				this->packetWaitingAck[this->base % seq_len].seqnum = -1;
				this->base++;
			}
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
	}
}

void SRRdtSender::timeoutHandler(int seqNum)
{
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	pns->stopTimer(SENDER, seqNum);							  // ���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); // �����������ͷ���ʱ��
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�����", this->packetWaitingAck[seqNum % seq_len]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % seq_len]); // ���·������ݰ�
}
