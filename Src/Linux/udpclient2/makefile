# makefile
.SUFFIXES : .cpp .o


OBJECTS = ../netcom/network.o ../netcom/netlauncher.o ../netcom/packetqueue.o ../netcom/tcpserver.o ../netcom/udpserver.o ../netcom/udpclient.o  ../opencvcom/streamingsender.o  main.o
SRCS = $(OBJECTS:.o=.cpp)


CC = g++
CXXFLAGS = -g -std=c++11 -Wall -pedantic -pthread 
TARGET = udpclient2


$(TARGET) : $(OBJECTS)
	$(CC) -pthread -o $@  $(OBJECTS) `pkg-config --cflags --libs opencv`

.cpp.o :
	$(CC) $(CXXFLAGS) -c $< -o $@

clean : 
	$(RM) -f $(OBJECTS) $(TARGET) core

