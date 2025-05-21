@echo off
:: appname 程序名
:: inputname 输入文件名
:: outputname 输出文件名
:: resultname 程序控制台输出重定向文件名

set appname="D:\\大三上\\计网实验\\hust_computer_network_experiment\\实验二\\EasyTCP\\Debug\\StopWait.exe"
set inputname="D:\\大三上\\计网实验\\hust_computer_network_experiment\\实验二\\input.txt"
set outputname="D:\\大三上\\计网实验\\hust_computer_network_experiment\\实验二\\output.txt"
set resultname="result.txt"

for /l %%i in (1,1,10) do (
    echo Test %appname% %%i:
    %appname% > %resultname% 2>&1
    fc /N %inputname% %outputname%
)
pause