@echo off
:: appname ������
:: inputname �����ļ���
:: outputname ����ļ���
:: resultname �������̨����ض����ļ���

set appname="D:\\������\\����ʵ��\\hust_computer_network_experiment\\ʵ���\\EasyTCP\\Debug\\StopWait.exe"
set inputname="D:\\������\\����ʵ��\\hust_computer_network_experiment\\ʵ���\\input.txt"
set outputname="D:\\������\\����ʵ��\\hust_computer_network_experiment\\ʵ���\\output.txt"
set resultname="result.txt"

for /l %%i in (1,1,10) do (
    echo Test %appname% %%i:
    %appname% > %resultname% 2>&1
    fc /N %inputname% %outputname%
)
pause