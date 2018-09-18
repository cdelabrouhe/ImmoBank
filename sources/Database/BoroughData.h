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
	unsigned int		m_key = 0xffffffff;
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

	std::string ComputeRequestURL() const;

	static bool compare(const BoroughData &_a, const BoroughData &_b)
	{
		return _a.m_name < _b.m_name;
	}

private:
	int m_httpRequestID = -1;
};