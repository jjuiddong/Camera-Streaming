//
// 2016-02-06, jjuiddong
//
// gcc용 network 전역 선언
//
//
#pragma once


#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <algorithm>



// 매크로 정의
#ifndef SAFE_DELETE
	#define SAFE_DELETE(p) {if (p) { delete p; p=NULL;} }
#endif
#ifndef SAFE_DELETEA
	#define SAFE_DELETEA(p) {if (p) { delete[] p; p=NULL;} }
#endif
#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p) {if (p) { p->Release(); p=NULL;} }
#endif
#ifndef SAFE_RELEASE2
	#define SAFE_RELEASE2(p) {if (p) { p->release(); p=NULL;} }
#endif
#ifndef IN
	#define IN
#endif
#ifndef OUT
	#define OUT
#endif
#ifndef INOUT
	#define INOUT
#endif

#define RET(exp)		{if((exp)) return; }			// exp가 true이면 리턴
#define RETV(exp,val)	{if((exp)) return val; }
#define ASSERT_RET(exp)		{assert(exp); RET(!(exp) ); }
#define ASSERT_RETV(exp,val)		{assert(exp); RETV(!(exp),val ); }
#define BRK(exp)		{if((exp)) break; }			// exp가 break
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)


typedef unsigned int    u_int;
typedef int SOCKET;
typedef unsigned char BYTE;
typedef unsigned int DWORD;

using std::vector;
using std::map;
using std::string;

namespace network {
	enum {
		BUFFER_LENGTH = 512,
	};

	struct fd_array : fd_set
	{
		SOCKET fd_array[128];
		int fd_count;
	};
}

namespace
{
	class __GET_TICK_COUNT
	{
	public:
		__GET_TICK_COUNT()
		{
			if (gettimeofday(&tv_, NULL) != 0)
				throw 0;
		}
		timeval tv_;
	};
	__GET_TICK_COUNT timeStart;
}

unsigned long GetTickCount();
unsigned long GetTickCount2();
std::string GetHostIP(const int networkCardIndex);


#include "common.h"
#include "autocs.h"
#include "session.h"
#include "packetqueue.h"
#include "tcpserver.h"
#include "tcpclient.h"
#include "udpclient.h"
#include "udpserver.h"
