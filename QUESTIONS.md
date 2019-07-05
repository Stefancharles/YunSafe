#Quetions of IDE
Keil 的MDK5 界面古老
而Clion也支持嵌入式开发
有友好的界面，智能提示，
也有debug功能
使用STM32CubeMX Generate Code
再用Clion打开项目代码
配置环境后就可以编辑
建立工程非常方便

#Questions of LIBS
STM32 有 HAL库、标准外设库、LL库
HAL和STD不兼容
在STM32CubeMX中官方已经放弃对STD库的支持
所以只能生成HAL库的代码
而正点原子给的示例代码是STD库
这会导致很多麻烦
问题是
在保险箱的代码中我看到了WiFi模块是用的HAL
然而其他一些地方又不是如此

#Questions of encode
保险箱项目里有的文件代码使用UTF8
有的使用GBK。。。
更改编码方式又导致另一个文件乱码
导致中文注释或者一些地方乱码
很不方便阅读代码
希望大家统一编码UTF8
