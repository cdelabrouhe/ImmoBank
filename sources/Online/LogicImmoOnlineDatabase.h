#pragma once

#include "OnlineDatabase.h"

namespace ImmoBank
{
	class LogicImmoOnlineDatabase : public OnlineDatabase
	{
	private:
		struct sAnnonce
		{
			bool Serialize(const std::string& _str);

			std::string m_city;
			std::string m_name;
			std::string m_description;
			std::string m_URL;
			std::string m_imageURL;
			Category m_category;
			int m_price;
			float m_surface;
			int m_nbRooms;
			int m_nbBedRooms;
			int	m_ID;
		};

		struct sRecherche
		{
			void Serialize(const std::string& _str);

			std::string m_resume;
			int m_nbAnnonces;

			std::vector<sAnnonce> m_annonces;
		};

	public:
		virtual void Init() override;
		virtual int SendRequest(SearchRequest* _request) override;

	protected:
		virtual bool ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) override;
	};
}