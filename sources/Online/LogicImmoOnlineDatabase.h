#pragma once

#include "OnlineDatabase.h"

namespace ImmoBank
{
	class LogicImmoOnlineDatabase : public OnlineDatabase
	{
		struct sLocalData : public EntryData
		{
			virtual ~sLocalData()	{}

			virtual void Generate(DatabaseHelper* _db) override;
			virtual void Load(DatabaseHelper* _db) override;
			virtual void copyTo(EntryData* _target) override;

			std::string m_cityName;
			std::string m_key;
			int m_zipCode = -1;
		};

	public:
		virtual void Init() override;
		virtual void Process() override;
		virtual int SendRequest(SearchRequest* _request) override;

		virtual void ReferenceCity(const std::string& _name) override;
		virtual void ReferenceBorough(const BoroughData& _borough) override;
		virtual bool HasCity(const std::string& _name, const int _zipCode, sCity& _city) override;

		EntryData* GetEntryData(const std::string& _cityName, const int _zipCode) const;

		std::string GetKey(sCity& _city) const;

	protected:
		virtual bool _ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) override;
		virtual std::string _ComputeKeyURL(const std::string& _name) override;

		virtual EntryData* _GenerateEntryData() override;
		virtual void _DecodeData(const std::string& _data, const sBoroughData& _sourceBorough) override;

		void _UpdateData(const std::string& _cityName, const int _zipCode, const std::string& _key);
		virtual EntryData* _GetEntryDataFromSource(EntryData* _source) const override;
		virtual EntryData* _GetEntryDataFromFullKey(void* _key) const override;
		virtual EntryData* _GetEntryDataFromCityName(const std::string& _name) const override;

	private:
		int		m_currentKeyID = -1;
	};
}