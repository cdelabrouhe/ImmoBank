#pragma once

#include <string>
#include <vector>
#include "extern/jsoncpp/value.h"

namespace ImmoBank
{
	struct sPrice;

	class Tools
	{
	public:

		static bool m_devMode;

		static bool IsDevMode();
		static void SetDevMode(bool _state);
		static void InvertDevMode();

		enum SortType
		{
			Rate,
			Price,
			Surface,
			PriceM2,
			COUNT
		};

		static float ComputeRentabilityRate(float _rent, float _price);
		static bool ExtractPricesFromHTMLSource(const std::string& _source, sPrice& _rentT1, sPrice& _rentT2, sPrice& _rentT3, sPrice& _rentT4Plus, sPrice& _buyApartment, sPrice& _buyHouse, unsigned int& _meilleursAgentsKey);
		static std::string GetExePath();
		static bool ReadJSON(const char* _path, Json::Value& _data);
		static bool WriteJSON(const char* _path, Json::Value& _data);
		static unsigned char* Read(const char* _path, int& _size);
		static bool Write(const char* _path, unsigned char* _buffer, int _size);
		static bool FileExists(const char* _path);
		static void DeleteFileOnDisk(const char* _path);
		static float GetVersionNumber();

		static int reduce(int gap)
		{
			gap = gap * 10 / 13;
			if (gap == 9 || gap == 10)
				gap = 11;
			if (gap < 1)
				return 1;
			return gap;
		}

		template <typename T>
		static void DoboSort(std::vector<T*>& tab, SortType _sortType)
		{
			int size = (int)tab.size();
			int gap = size;
			bool swapped;
			do {
				swapped = false;
				gap = reduce(gap);
				for (int i = 0; i < size - gap; i++)
				{
					if (T::compare(tab[i], tab[i + gap], _sortType))
					{
						swapped = true;
						T* tmp = tab[i];
						tab[i] = tab[i + gap];
						tab[i + gap] = tmp;
					}
				}
			} while (gap > 1 || swapped);
		}
	};
}