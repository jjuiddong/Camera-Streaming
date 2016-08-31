# makefile
.SUFFIXES : .cpp .o


OBJECTS = ../netcom/network.o ../netcom/netlauncher.o ../netcom/session.o ../netcom/packetqueue.o ../netcom/tcpclient.o main.o
SRCS = $(OBJECTS:.o=.cpp)


CC = g++
CXXFLAGS = -g -std=c++11 -Wall -pedantic -pthread
TARGET = tcpchatclient


$(TARGET) : $(OBJECTS)
	$(CC) -pthread -o $@  $(OBJECTS)

.cpp.o :
	$(CC) $(CXXFLAGS) -c $< -o $@

clean : 
	$(RM) -f $(OBJECTS) $(TARGET) core

