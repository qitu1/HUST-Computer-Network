// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"


int main(int argc, char* argv[])
{
	freopen("D:/������/����ʵ��/hust_computer_network_experiment/ʵ���/EasyTCP/EasyTCPlog.txt", "w", stdout);
	//freopen("C:\\Users\\lenovo\\Desktop\\result.txt", "w", stdout);
	RdtSender *ps = new EasyTCPRdtSender();
	RdtReceiver * pr = new EasyTCPRdtReceiver();
//	pns->setRunMode(0);  //VERBOSģʽ
	pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("D:/������/����ʵ��/hust_computer_network_experiment/ʵ���/input.txt");
	pns->setOutputFile("D:/������/����ʵ��/hust_computer_network_experiment/ʵ���output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	
	return 0;
}

