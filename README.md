
##### 前言

本demo是使用的mp4v2来将mp4文件解封装得到h264、aac的，目前demo提供的.a静态库文件是在x86_64架构的Ubuntu16.04编译得到的，如果想在其他环境下测试demo，可以自行编译mp4v2并替换相应的库文件（libmp4v2.a）。


###  1. 编译

```bash
$ make # 或者`make DEBUG=1`打开调试打印信息
```

如果想编译mp4v2，则可以参考以下步骤：

mp4v2源码下载地址：[https://github.com/TechSmith/mp4v2](https://github.com/TechSmith/mp4v2)

```bash
$ tar xjf mp4v2-2.0.0.tar.bz2
$ cd mp4v2-2.0.0/
$ ./configure --prefix=$PWD/_install # 交叉编译可参考加上选项: --host=arm-linux-gnueabihf
$ make -j96
$ make install
```


### 2. 使用

注：示例2中的音视频测试源文件是不同步的，不影响本demo的解封装。

```bash
$ ./mp4v2_unpack_demo 
Usage: 
   ./mp4v2_unpack_demo ./avfile/test1.mp4 ./test1_out.h264 ./test1_out.aac
   ./mp4v2_unpack_demo ./avfile/test2.mp4 ./test2_out.h264 ./test2_out.aac
```

### 3. 参考文章

 - [01.mp4v2应用—mp4转h264 - wade_linux - 博客园.mhtml](https://www.cnblogs.com/Lwd-linux/p/7390951.html)

 - [mp4文件格式解析 - 简书](https://www.jianshu.com/p/529c3729f357)

 - [MP4格式详解_DONGHONGBAI的专栏-CSDN博客](https://blog.csdn.net/DONGHONGBAI/article/details/84401397)

 - [使用mp4v2解码mp4转成h264码流和aac码流_lq496387202的博客-CSDN博客_mp4v2解码](https://blog.csdn.net/lq496387202/article/details/81510622)


### 附录（demo目录结构）

```
.
├── avfile
│   ├── test1.mp4
│   ├── test1_out.aac
│   ├── test1_out.h264
│   ├── test2.mp4
│   ├── test2_out.aac
│   └── test2_out.h264
├── docs
│   ├── 01.mp4v2应用—mp4转h264 - wade_linux - 博客园.mhtml
│   ├── mp4文件格式解析 - 简书.mhtml
│   ├── MP4格式详解_DONGHONGBAI的专栏-CSDN博客.mhtml
│   └── 使用mp4v2解码mp4转成h264码流和aac码流_lq496387202的博客-CSDN博客_mp4v2解码.mhtml
├── include
│   └── mp4v2
│       ├── chapter.h
│       ├── file.h
│       ├── file_prop.h
│       ├── general.h
│       ├── isma.h
│       ├── itmf_generic.h
│       ├── itmf_tags.h
│       ├── mp4v2.h
│       ├── platform.h
│       ├── project.h
│       ├── sample.h
│       ├── streaming.h
│       ├── track.h
│       └── track_prop.h
├── lib
│   └── libmp4v2.a
├── main.c
├── Makefile
└── README.md
```

