// StopWait.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"


int main(int argc, char* argv[])
{
	freopen("D:/大三上/计网实验/hust_computer_network_experiment/实验二/EasyTCP/EasyTCPlog.txt", "w", stdout);
	//freopen("C:\\Users\\lenovo\\Desktop\\result.txt", "w", stdout);
	RdtSender *ps = new EasyTCPRdtSender();
	RdtReceiver * pr = new EasyTCPRdtReceiver();
//	pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("D:/大三上/计网实验/hust_computer_network_experiment/实验二/input.txt");
	pns->setOutputFile("D:/大三上/计网实验/hust_computer_network_experiment/实验二output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;
}

