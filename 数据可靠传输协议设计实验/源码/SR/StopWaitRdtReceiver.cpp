#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtReceiver.h"

SRRdtReceiver::SRRdtReceiver() : base(0), expectSequenceNumberRcvd(0)
{
	lastAckPkt.acknum = -1; // ��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1; // ���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
	{
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < 2 * Configuration::WINDOW_LEN; i++)
	{
		this->packetReceived[i] = false;
		this->cachedPkt[i].acknum = -1;
		this->cachedPkt[i].seqnum = -1;
	}
}

SRRdtReceiver::~SRRdtReceiver()
{
}

void SRRdtReceiver::receive(const Packet &packet)
{
	const int &window_len = Configuration::WINDOW_LEN; // ���ڴ�С��д
	const int seq_len = 2 * Configuration::WINDOW_LEN;
	// ���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	// ��ӡ���շ�����
	cout << "���շ�����ǰ����: { ";
	for (int i = this->base; i < window_len + this->base; i++)
	{
		if (this->packetReceived[i % seq_len])
		{
			cout << i << ' ';
		}
		else
		{
			cout << '_' << ' ';
		}
	}
	cout << "}" << endl;
	// ���У�����ȷ
	if (checkSum == packet.checksum)
	{
		if (packet.seqnum < window_len + base && base < packet.seqnum) // �յ�ʧ����
		{
			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
			this->cachedPkt[packet.seqnum % seq_len] = packet;
			this->packetReceived[packet.seqnum % seq_len] = true;
			// ���� ACK
			lastAckPkt.acknum = packet.seqnum; // ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt); // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
		else if (base == packet.seqnum) // �յ�������
		{
			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
			this->cachedPkt[packet.seqnum % seq_len] = packet;
			this->packetReceived[packet.seqnum % seq_len] = true;
			this->cachedPkt[packet.seqnum % seq_len].acknum = 0;
			// ���� ACK
			lastAckPkt.acknum = packet.seqnum; // ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt); // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
			// ��������
			while (this->packetReceived[this->base % seq_len] == true)
			{
				// ȡ��Message�����ϵݽ���Ӧ�ò�
				Message msg;
				memcpy(msg.data, cachedPkt[this->base % seq_len].payload, sizeof(cachedPkt[this->base % seq_len].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				this->packetReceived[this->base % seq_len] = false;
				this->cachedPkt[packet.seqnum % seq_len].acknum = -1;
				this->base++;
			}
		}
		else if (packet.seqnum < base && packet.seqnum >= base - window_len) //�յ��Ѿ�ȷ�ϵı���
		{
			pUtils->printPacket("���շ��յ��ظ�����", packet);
			lastAckPkt.acknum = packet.seqnum; // ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt); // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
		else
		{
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			cout << "���շ��Ļ������ڣ�" << base << " & " << window_len + base << endl;
		}
	}
	else
	{
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
	}
}