#pragma once

#include "SearchRequestResult.h"
#include "SearchRequestAnnounce.h"

namespace ImmoBank
{
	struct SearchRequestResultAnnounce : public SearchRequestResult
	{
		SearchRequestResultAnnounce() : SearchRequestResult(SearchRequestType_Announce) {}
		SearchRequestResultAnnounce(SearchRequestAnnounce& _request) : SearchRequestResult(SearchRequestType_Announce)
		{
			*this = _request;
		}

		std::string m_database;
		std::string m_name;
		std::string m_description;
		std::string m_URL;
		std::string m_imageURL;
		std::string m_imageTinyURL;
		std::string m_loadedImageURL;
		sCity		m_city;
		Type		m_type = Type_NONE;
		Category	m_category = Category_NONE;
		BoroughData	m_selectedBorough;
		std::vector<BoroughData> m_boroughs;
		int			m_price = 0;
		int			m_priceM2 = 0;
		float		m_surface = 0.f;
		int			m_rent = 0;
		int			m_nbRooms = 0;
		int			m_nbBedRooms = 0;
		int			m_selectedBoroughID = 0;
		int			m_inseeCode = 0;
		int			m_imageDownloadRequestID = -1;
		unsigned int	m_imageTextureID = 0;
		bool		m_imageDownloaded = false;

		virtual void Init() override;
		virtual void End() override;

		virtual void PostProcess() override;
		virtual bool Display(ImGuiTextFilter* _filter = nullptr) override;
		float GetRentabilityRate() const;
		float GetEstimatedRent() const;

		SearchRequestResultAnnounce& SearchRequestResultAnnounce::operator=(const SearchRequestAnnounce &_request)
		{
			m_city = _request.m_city;
			m_type = _request.m_type;
			return *this;
		}

		virtual bool Compare(const SearchRequestResult* _target, Tools::SortType _sortType, bool _invert) const override;

	private:
		void UpdateBoroughs(bool _lookForBorough);
		bool _LoadImage(const std::string& _path, int& _requestID, unsigned int& _textureID, bool& _downloadStatus);

	private:
		bool m_waitingForDBUpdate = false;
	};
}