TRAGET := mp4v2_unpack_demo

CXX := g++
CFLAG := -I./include

# 控制程序里的DEBUG打印开关
ifeq ($(DEBUG), 1)
CFLAG += -DENABLE_DEBUG
endif


all : $(TRAGET) $(TRAGET2)

$(TRAGET) : main.c lib/libmp4v2.a
	$(CXX) $^ $(CFLAG) -o $@

clean : 
	rm -rf $(TRAGET) *.h264 *.264 *.aac
.PHONY := clean

