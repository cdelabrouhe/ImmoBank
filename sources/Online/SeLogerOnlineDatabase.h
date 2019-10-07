#pragma once

#include "OnlineDatabase.h"

namespace ImmoBank
{
	class SeLogerOnlineDatase : public OnlineDatabase
	{
	private:
		struct sAnnonce
		{
			void Serialize(const std::string& _str);

			std::string m_name;
			std::string m_description;
			std::string m_URL;
			std::string m_imageURL;
			std::string m_imageTinyURL;
			Category m_category;
			int m_price;
			float m_surface;
			int m_nbRooms;
			int m_nbBedRooms;
			int	m_ID;
			int m_inseeCode;
		};

		struct sSummary
		{
			void Serialize(const std::string& _str);

			std::string m_resume;
			std::string m_resumeSansTri;
			int m_nbAnnonces;
			int m_nbAnnoncesAffichables;

			std::vector<sAnnonce> m_annonces;
		};

		struct sRecherche
		{
			void Serialize(const std::string& _str);

			sSummary	m_summary;
		};

	public:
		virtual void Init() override;
		virtual int SendRequest(SearchRequest* _request) override;

	protected:
		virtual bool ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) override;
	};
}