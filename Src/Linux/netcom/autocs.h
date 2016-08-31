//
// criticalsection 객체를 자동으로 제거하는 클래스다.
//
// 2016-02-06
//		- gcc 용 작업
//
#pragma once




class cAutoCS
{
public:
	cAutoCS(pthread_mutex_t &cs) :
		m_cs(cs)
		, m_isLeave(false)
	{
		pthread_mutex_lock(&cs);
	}

	virtual ~cAutoCS()
	{
 		if (!m_isLeave)
 			pthread_mutex_unlock(&m_cs);
		m_isLeave = true;
	}

	void Enter()
	{
 		if (m_isLeave)
			pthread_mutex_lock(&m_cs);
		m_isLeave = false;
	}

	void Leave()
	{
		pthread_mutex_unlock(&m_cs);
		m_isLeave = true;
	}

	pthread_mutex_t &m_cs;
	bool m_isLeave;
};

