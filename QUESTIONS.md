## 学习总结
>* 经过这五天的学习，我学会了使用keil 软件新建工程，怎么把stm32的相关启动文件加到工程里，把库文件加到项目。

>* 我比较喜欢用clion作为开发工具，所以这期间自己探索了一下怎么使用时stm32cubemx和clion联合编辑工程，但是中间遇到了库和目前正点原子提供的库不兼容的问题，调用的库函数名称和参数都和学习的视频里不一样。我目前的想法是先按照正点提供的库来学习基础的库函数，等熟悉后再使用ll,hal库。

>* 目前自学了led，按键，蜂鸣器，gpio,串口通信，中断部分。这很多是和51有相似的地方，只是控制相关资源的位数增加了，封装成函数，使开发相对容易简单一些。但是对于其他通信模块我很没有底，不知道该从整体上如何与云平台进行通信。


## Quetions of IDE

>* keil 的MDK5 界面古老,而Clion也支持嵌入式开发,有友好的界面，智能提示，也有debug功能.使用STM32CubeMX Generate Code,再用Clion打开项目代码,配置环境后就可以编辑.建立工程非常方便

## Questions of LIBS

>* STM32 有 HAL库、标准外设库、LL库
HAL和STD不兼容
在STM32CubeMX中官方已经放弃对STD库的支持
所以只能生成HAL库的代码
而正点原子给的示例代码是STD库
这会导致很多麻烦
问题是
在保险箱的代码中我看到了WiFi模块是用的HAL
然而其他一些地方又不是如此

## Questions of encode

>* 保险箱项目里有的文件代码使用UTF8
有的使用GBK。。。
更改编码方式又导致另一个文件乱码
导致中文注释或者一些地方乱码
很不方便阅读代码
希望大家统一编码UTF8
