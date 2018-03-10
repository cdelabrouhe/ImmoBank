#pragma once

#include <assert.h>

#define ASSERT assert

#ifdef WIN32
typedef unsigned int			ux;
typedef int						ix;
typedef signed long long		s64;
typedef unsigned long long		u64;
#else
typedef unsigned long long int	ux;
typedef long long int			ix;
typedef signed long				s64;
typedef unsigned long			u64;
#endif

#define START_YEAR		2018

struct sDate
{
public:
	sDate()
	{
		SetData(0);
	}

	void SetDate(int _year, int _month, int _day, int _hour, int _minute, int _second)
	{
		m_year = _year - START_YEAR;
		m_month = _month;
		m_day = _day;
		m_hour = _hour;
		m_minute = _minute;
		m_second = _second;
	}
	void SetData(unsigned int _data)
	{
		m_data = _data;
	}
	unsigned int GetData() const
	{
		return m_data;
	}
	void GetDate(int& _year, int& _month, int& _day, int& _hour, int& _minute, int& _second)
	{
		_year = m_year + START_YEAR;
		_month = m_month;
		_day = m_day;
		_hour = m_hour;
		_minute = m_minute;
		_second = m_second;
	}

	int GetYear()		{ return m_year + START_YEAR; }
	int GetMonth()		{ return m_month; }
	int GetDay()		{ return m_day; }
	int GetHour()		{ return m_hour; }
	int GetMinute()		{ return m_minute; }
	int GetSecond()		{ return m_second; }

private:
	union
	{
		struct
		{
			unsigned int		m_year : 6;			// 2018 + m_year
			unsigned int		m_month : 4;
			unsigned int		m_day : 5;
			unsigned int		m_hour : 5;
			unsigned int		m_minute : 6;
			unsigned int		m_second : 6;
		};

		unsigned int		m_data;
	};
};