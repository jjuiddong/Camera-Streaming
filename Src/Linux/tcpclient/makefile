# makefile
.SUFFIXES : .cpp .o


OBJECTS = tcpclientmain.o
SRCS = $(OBJECTS:.o=.cpp)


CC = g++
CXXFLAGS = -g
TARGET = tcpclient


$(TARGET) : $(OBJECTS)
	$(CC) -o $@  $(OBJECTS)

.cpp.o :
	$(CC) $(CXXFLAGS) -c $< -o $@

clean : 
	$(RM) -f $(OBJECTS) $(TARGET) core

