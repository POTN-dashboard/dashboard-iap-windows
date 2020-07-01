# dashboard-iap-windows
Windows client for Dashboard-IAP（BOOTLOADER）
When you need use this function. You should use the dashboard-iap-stm32 to create binary file. 
The binary file's name should rename POTN and copy to dashboard-iap-windows's exe file folder.
When you run this iap，after about 10s you should pull out this device and insert it again.
（There are a bug.I can't solve it.Insert again can solve this problem）


使用IAP在线升级时， 你需要用dashboard-iap-stm32生成二进制文件
把生成的二进制文件重命名为POTN后放入dashboard-iap-windows的exe同一文件夹下
你开始使用IAP功能时，大概十秒钟后，你需要重新插拔设备
（这是一个bug，我还不能解决它，重新插拔可以解决这个问题）

bug原因：stm32 从 app程序跳转到 iap程序后（这里利用的是直接软件复位跳转），STM32的USB不能重新被识别。
但是从iap跳转到app后，usb可以重新被识别。目前本菜鸟还不能通过软件解决它。
