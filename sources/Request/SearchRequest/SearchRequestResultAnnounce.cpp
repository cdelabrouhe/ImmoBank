#include "SearchRequestResultAnnounce.h"
#include "Tools\StringTools.h"
#include "extern/ImGui/imgui.h"
#include <windows.h>
#include <shellapi.h>
#include <algorithm>

void SearchRequestResultAnnounce::PostProcess()
{
	StringTools::RemoveSpecialCharacters(m_name);
	StringTools::RemoveSpecialCharacters(m_description);

	DatabaseManager::getSingleton()->GetBoroughs(m_city, m_boroughs);

	std::sort(m_boroughs.begin(), m_boroughs.end(), [](BoroughData& a, BoroughData& b) {
		return a.m_name < b.m_name;
	});

	int ID = 0;
	for (auto& borough : m_boroughs)
	{
		auto findIndex = m_description.find(borough.m_name);
		if (findIndex != std::string::npos)
		{
			m_selectedBorough = borough;
			m_selectedBoroughID = ID + 1;
			break;
		}

		++ID;
	}
}

void SearchRequestResultAnnounce::Display()
{
	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::Separator();
	ImGui::SetWindowFontScale(1.2f);
	std::string name = (m_category == Category_Apartment ? "Apartment" : "House");
	name += ", " + m_name;
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), name.c_str());
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
		ShellExecuteA(NULL, "open", m_URL.c_str(), NULL, NULL, SW_SHOWDEFAULT);

	ImGui::SetWindowFontScale(1.0f);

	ImGui::TextWrapped(m_description.c_str());

	ImGui::Columns(6);

	ImGui::Separator();

	std::vector<const char*> data;
	data.resize(m_boroughs.size() + 1);
	data[0] = "";
	for (auto ID = 0; ID < m_boroughs.size(); ++ID)
		data[ID + 1] = m_boroughs[ID].m_name.c_str();
	ImGui::PushID(this + 12345);
	ImGui::Combo("Borough", &m_selectedBoroughID, data.data(), (int)data.size());
	ImGui::PopID();

	// Compute rentability rate
	BoroughData& borough = m_boroughs[m_selectedBoroughID];
	float rent = 0.f;
	if (m_nbRooms == 1)
		rent = m_surface * borough.m_priceRentApartmentT1.m_val;
	else if (m_nbRooms == 2)
		rent = m_surface * borough.m_priceRentApartmentT2.m_val;
	else if (m_nbRooms == 3)
		rent = m_surface * borough.m_priceRentApartmentT3.m_val;
	else
		rent = m_surface * borough.m_priceRentApartmentT4Plus.m_val;

	ImGui::NextColumn();
	ImGui::Text("Price: %u     ", m_price);
	ImGui::NextColumn();
	ImGui::Text("Surface: %.0f m2     ", m_surface);
	ImGui::NextColumn();
	ImGui::Text("Nb rooms: %u     ", m_nbRooms);
	ImGui::NextColumn();
	ImGui::Text("Nb bedrooms: %u     ", m_nbBedRooms);
	ImGui::NextColumn();

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::Text(" ");
}