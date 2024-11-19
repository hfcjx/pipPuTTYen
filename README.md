# pipPuTTYen
PuTTY(英文便携版)的分支上扩展功能， https://jakub.kotrla.net/putty/

新增功能:

1) 利用std-in，std-out，std-err与PuTTYAttach交换数据。

   std-in 接收PuTTYAttach数据发送到远端。
   
   std-out 接收远端数据发送到PuTTYAttach,只用于文件传输
   
   std-err 发送PuTTY日志数据到PuTTYAttach。
   
3) 增加Windows消息WM_COPYDATA，用于与外部程序交互状态数据。
4) 增加Windows消息WM_DROPFILES接收文件拖放，与WM_COPYDATA配合，实现文件拖放上传。


## 文件传输

要使用XModem，YModem，ZModem协议传输文件，需要以下两个外部程序协同工作。

[PuTTYAttach](https://github.com/hfcjx/PuTTYAttach)

[lrzsz-pip](https://github.com/hfcjx/lrzsz-pip) 或 [lrzsz-win32](https://github.com/trzsz/lrzsz-win32)

## MIT 许可证
PuTTY 采用不同于 GNU 软件的 MIT 许可证。简单的说，MIT 许可证比 GPL 更加自由。类似与 BSD 许可证，但也比 BSD 许可证更为宽松。对于 PuTTY 中文版版权页面 MIT 许可证翻译文本仅供参考，一切法律效力以[英文原本](http://www.chiark.greenend.org.uk/~sgtatham/putty/licence.html)为准。

