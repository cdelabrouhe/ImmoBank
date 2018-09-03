#include "TextManager.h"

TextManager* s_singleton = nullptr;

//-------------------------------------------------------------------------------------------------
TextManager* TextManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new TextManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
void TextManager::Init()
{

}

//-------------------------------------------------------------------------------------------------
void TextManager::End()
{

}
