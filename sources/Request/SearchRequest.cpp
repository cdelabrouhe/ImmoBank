#include "SearchRequest.h"
#include <Tools/StringTools.h>
#include "extern/ImGui/imgui.h"
#include <windows.h>
#include <shellapi.h>

void SearchRequestResultAnnounce::PostProcess()
{
	StringTools::RemoveSpecialCharacters(m_name);
	StringTools::RemoveSpecialCharacters(m_description);
}

void SearchRequestResultAnnounce::Display()
{
	ImGui::Separator();
	ImGui::SetWindowFontScale(1.2f);
	std::string name = (m_category == Category_Apartment ? "Apartment" : "House");
	name += ", " + m_name;
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), name.c_str());
	ImGui::SetWindowFontScale(1.0f);

	ImGui::TextWrapped(m_description.c_str());
	ImGui::Text("Price: %u", m_price);
	ImGui::Text("Surface: %.0f", m_surface);
	ImGui::Text("Nb rooms: %u", m_nbRooms);
	ImGui::Text("Nb bedrooms: %u", m_nbBedRooms);
	ImGui::TextColored(ImVec4(0.2f, 0.75f, 1.0f, 1.0f), "URL (%s)", m_database.c_str());
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
		ShellExecuteA(NULL, "open", m_URL.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

void SearchRequest::copyTo(SearchRequest* _target)
{
	_target->m_requestType = m_requestType;
}

void SearchRequestAnnounce::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	SearchRequestAnnounce* target = (SearchRequestAnnounce*)_target;
	target->m_city = m_city;
	target->m_type = m_type;
	target->m_categories = m_categories;
	target->m_priceMin = m_priceMin;
	target->m_priceMax = m_priceMax;
	target->m_surfaceMin = m_surfaceMin;
	target->m_surfaceMax = m_surfaceMax;
	target->m_nbRooms = m_nbRooms;
	target->m_nbBedRooms = m_nbBedRooms;
}