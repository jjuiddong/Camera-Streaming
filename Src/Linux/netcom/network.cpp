
#include "network.h"


unsigned long GetTickCount()
{
	static time_t   secStart = timeStart.tv_.tv_sec;
	static time_t   usecStart = timeStart.tv_.tv_usec;
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - secStart) * 1000 + (tv.tv_usec - usecStart) / 1000;
}

unsigned long GetTickCount2()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	return (unsigned long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}


// 현재 컴퓨터의 IP Address를 리턴한다.
string GetHostIP(const int networkCardIndex)
{
	FILE *fp = popen("ip route get 1 | awk '{print $NF; exit}'", "r");
	char buff[256];
	fgets(buff, sizeof(buff), fp);
	pclose(fp);
	return buff;
}
