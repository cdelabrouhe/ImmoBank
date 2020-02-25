#pragma once

#include <assert.h>
#include <string>

#define ASSERT assert

#define DISABLE_OPTIMIZE	__pragma(optimize("",off))
#define ENABLE_OPTIMIZE	__pragma(optimize("",on))

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

namespace ImmoBank
{
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

		int GetYear() { return m_year + START_YEAR; }
		int GetMonth() { return m_month; }
		int GetDay() { return m_day; }
		int GetHour() { return m_hour; }
		int GetMinute() { return m_minute; }
		int GetSecond() { return m_second; }

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

	struct sCity
	{
		sCity(const std::string& _name = ""
				, int _codeInsee = 0
				, int _zipCode = 0
				, const std::string& _logicImmoKey = ""
				, unsigned int _papKey = 0)
			: m_name(_name)
			, m_inseeCode(_codeInsee)
			, m_zipCode(_zipCode)
			, m_logicImmoKey(_logicImmoKey)
			, m_papKey(_papKey)
		{}

		std::string		m_name;
		std::string		m_logicImmoKey;
		int				m_inseeCode;
		int				m_zipCode;
		unsigned int	m_papKey = 0xFFFFFFFF;

		void FixName();
		void UnFixName();
		static void FixName(std::string& _name);
		static void UnFixName(std::string& _name);

		static bool compare(const sCity &_a, const sCity &_b)
		{
			return _a.m_name < _b.m_name;
		}
	};
}