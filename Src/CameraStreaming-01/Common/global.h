#pragma once


namespace common
{
	namespace dbg {
		static void Log(const char* fmt, ...) {}
		static void ErrLog(const char* fmt, ...) {}
	}


	class cAutoCS
	{
	public:
		cAutoCS(CRITICAL_SECTION &cs) :
			m_cs(cs)
			, m_isLeave(false)
		{
			EnterCriticalSection(&cs);
		}

		virtual ~cAutoCS()
		{
			if (!m_isLeave)
				LeaveCriticalSection(&m_cs);
			m_isLeave = true;
		}

		void Enter()
		{
			if (m_isLeave)
				EnterCriticalSection(&m_cs);
			m_isLeave = false;
		}

		void Leave()
		{
			LeaveCriticalSection(&m_cs);
			m_isLeave = true;
		}

		CRITICAL_SECTION &m_cs;
		bool m_isLeave;
	};


	/// Auto Lock, Unlock
	template<class T>
	class AutoLock
	{
	public:
		AutoLock(T& t) : m_t(t) { m_t.Lock(); }
		~AutoLock() { m_t.Unlock(); }
		T &m_t;
	};



	/// Critical Section auto initial and delete
	class CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();
		void Lock();
		void Unlock();
	protected:
		CRITICAL_SECTION m_cs;
	};

	inline CriticalSection::CriticalSection() {
		InitializeCriticalSection(&m_cs);
	}
	inline CriticalSection::~CriticalSection() {
		DeleteCriticalSection(&m_cs);
	}
	inline void CriticalSection::Lock() {
		EnterCriticalSection(&m_cs);
	}
	inline void CriticalSection::Unlock() {
		LeaveCriticalSection(&m_cs);
	}


	/// auto critical section lock, unlock
	class AutoCSLock : public AutoLock<CriticalSection>
	{
	public:
		AutoCSLock(CriticalSection &cs) : AutoLock(cs) { }
	};




	//------------------------------------------------------------------------
	// 유니코드를 멀티바이트 문자로 변환
	//------------------------------------------------------------------------
	static std::string wstr2str(const std::wstring &wstr)
	{
		const int slength = (int)wstr.length() + 1;
		const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, 0, 0, NULL, FALSE);
		char* buf = new char[len];
		::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, buf, len, NULL, FALSE);
		std::string r(buf);
		delete[] buf;
		return r;
	}


	//------------------------------------------------------------------------
	// 멀티바이트 문자를 유니코드로 변환
	//------------------------------------------------------------------------
	static std::wstring str2wstr(const std::string &str)
	{
		int len;
		int slength = (int)str.length() + 1;
		len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	//------------------------------------------------------------------------
	// 스트링포맷
	//------------------------------------------------------------------------
	static std::string format(const char* fmt, ...)
	{
		char textString[256] = { '\0' };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
		va_end(args);
		return textString;
	}



	//------------------------------------------------------------------------
	// 스트링포맷 wstring 용
	//------------------------------------------------------------------------
	static std::wstring formatw(const char* fmt, ...)
	{
		char textString[256] = { '\0' };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
		va_end(args);
		return str2wstr(textString);
	}



	// 현재 컴퓨터의 IP Address를 리턴한다.
	static string GetHostIP(const int networkCardIndex)
	{
		WSAData wsaData;
		if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
			return "";
		}

		char ac[80];
		if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
		{
			return "";
		}

		struct hostent *phe = gethostbyname(ac);
		if (phe == 0)
		{
			return "";
		}

		for (int i = 0; phe->h_addr_list[i] != 0; ++i)
		{
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));

			if (networkCardIndex == i)
				return inet_ntoa(addr);
		}

		WSACleanup();

		return "";
	}


	// elements를 회전해서 제거한다.
	template <class Seq>
	void rotatepopvector(Seq &seq, const unsigned int idx)
	{
		if ((seq.size() - 1) > idx)
			std::rotate(seq.begin() + idx, seq.begin() + idx + 1, seq.end());
		seq.pop_back();
	}


	// IPAddressCtrl의 IP정보를 문자열로 리턴한다.
	static string GetIP(CIPAddressCtrl &ipControl)
	{
		DWORD address;
		ipControl.GetAddress(address);
		std::stringstream ss;
		ss << ((address & 0xff000000) >> 24) << "."
			<< ((address & 0x00ff0000) >> 16) << "."
			<< ((address & 0x0000ff00) >> 8) << "."
			<< (address & 0x000000ff);
		const string ip = ss.str();
		return ip;
	}

}
