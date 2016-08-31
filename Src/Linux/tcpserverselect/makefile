# makefile
.SUFFIXES : .cpp .o


OBJECTS = tcpserverselectmain.o
SRCS = $(OBJECTS:.o=.cpp)


CC = g++
CXXFLAGS = -g
TARGET = tcpserverselect


$(TARGET) : $(OBJECTS)
	$(CC) -o $@  $(OBJECTS)

.cpp.o :
	$(CC) $(CXXFLAGS) -c $< -o $@

clean : 
	$(RM) -f $(OBJECTS) $(TARGET) core

