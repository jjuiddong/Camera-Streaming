# makefile
.SUFFIXES : .cpp .o


OBJECTS = tcpservermain.o
SRCS = $(OBJECTS:.o=.cpp)


CC = g++
CXXFLAGS = -g
TARGET = tcpserver


$(TARGET) : $(OBJECTS)
	$(CC) -o $@  $(OBJECTS)

.cpp.o :
	$(CC) $(CXXFLAGS) -c $< -o $@

clean : 
	$(RM) -f $(OBJECTS) $(TARGET) core

