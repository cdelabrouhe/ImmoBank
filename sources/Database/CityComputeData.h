#pragma once

#include <string>
#include <vector>
#include "BoroughData.h"

namespace ImmoBank
{
	class CityComputeData
	{
	public:
		enum UpdateStep
		{
			UpdateStep_NONE = -1,
			UpdateStep_GetCityData,
			UpdateStep_GetBoroughList,
			UpdateStep_ComputeBoroughsPrices,
			UpdateStep_COUNT
		};

		void Init();
		bool Process();
		void End();

		sCity						m_city;
		std::vector<BoroughData>	m_boroughs;
		int							m_boroughsListID = -1;

	private:
		void IncreaseStep() { m_state = (UpdateStep)((int)m_state + 1); }

	private:
		UpdateStep		m_state = UpdateStep_NONE;
	};
}