#pragma once

#include <string>
#include "Tools\Types.h"

struct sPrice
{
	sPrice(float _val = 0.f, float _min = 0.f, float _max = 0.f)
		: m_val(_val), m_min(_min), m_max(_max) {}

	void Reset() { m_min = 0.f; m_val = 0.f; m_max = 0.f; }

	float m_min = 0.f;
	float m_val = 0.f;
	float m_max = 0.f;
};

class BoroughData
{
public:
	BoroughData();

	std::string			m_name;
	sCity				m_city;
	sDate				m_timeUpdate;
	unsigned int		m_meilleursAgentsKey = 0xffffffff;
	unsigned int		m_selogerKey = 0;
	sPrice				m_priceRentApartmentT1;
	sPrice				m_priceRentApartmentT2;
	sPrice				m_priceRentApartmentT3;
	sPrice				m_priceRentApartmentT4Plus;
	sPrice				m_priceBuyApartment;
	sPrice				m_priceBuyHouse;
	sPrice				m_priceRentHouse;

	void Init();
	bool Process();
	void End();
	void Reset(bool _resetDB = false);

	void Edit();
	void DisplayAsTooltip();

	void SetWholeCity();
	bool IsWholeCity() const;

	void OpenInBrowser() const;

	bool IsValid() const;

	void SetTimeUpdateToNow();
	void SetSelogerKey(unsigned int _key, bool _isCity);
	int GetSelogerKey(bool* _isCity = nullptr);
	static unsigned int ConvertSelogerKey(unsigned int _key, bool _isCity);

	std::string ComputeRequestURL() const;
	std::string ComputeSeLogerKeyURL() const;

	bool operator==(const BoroughData &_other) const
	{
		return (_other.m_city.m_name == _other.m_city.m_name)
			&& (_other.m_name == _other.m_name)
			&& (_other.m_selogerKey == _other.m_selogerKey)
			&& (_other.m_meilleursAgentsKey == _other.m_meilleursAgentsKey);
	}

	static bool compare(const BoroughData &_a, const BoroughData &_b)
	{
		return _a.m_name < _b.m_name;
	}

private:
	int m_httpRequestID = -1;
};