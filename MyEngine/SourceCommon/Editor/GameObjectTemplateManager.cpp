//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

GameObjectTemplateManager::GameObjectTemplateManager()
{
    char* filestring = PlatformSpecific_LoadFile( "Data/DataEngine/EngineGameObjects.mytemplate", 0 );
    if( filestring == 0 )
        return;

    m_jRoot = cJSON_Parse( filestring );

    m_jRootTemplatesArray = cJSON_GetObjectItem( m_jRoot, "Templates" );
    AddTemplatesToVector( m_jRootTemplatesArray );

    delete[] filestring;
}

GameObjectTemplateManager::~GameObjectTemplateManager()
{
    cJSON_Delete( m_jRoot );
}

void GameObjectTemplateManager::AddTemplatesToVector(cJSON* jTemplateArray)
{
    for( int i=0; i<cJSON_GetArraySize( jTemplateArray ); i++ )
    {
        cJSON* jTemplate = cJSON_GetArrayItem( jTemplateArray, i );
        cJSON* jInnerArray = cJSON_GetObjectItem( jTemplate, "Templates" );

        GameObjectTemplate gotemplate;
        gotemplate.isfolder = (jInnerArray != 0); // it's a folder if there's an inner array
        gotemplate.jParent = jTemplateArray;
        gotemplate.jTemplate = jTemplate;
        m_jTemplates.push_back( gotemplate );

        if( jInnerArray )
        {
            AddTemplatesToVector( jInnerArray );
        }
    }
}

unsigned int GameObjectTemplateManager::GetNumberOfTemplates()
{
    return m_jTemplates.size();
}

bool GameObjectTemplateManager::IsTemplateAFolder(unsigned int templateid)
{
    return m_jTemplates[templateid].isfolder;
}

cJSON* GameObjectTemplateManager::GetTemplateJSONObject(unsigned int templateid)
{
    return m_jTemplates[templateid].jTemplate;
}

cJSON* GameObjectTemplateManager::GetParentTemplateJSONObject(unsigned int templateid)
{
    return m_jTemplates[templateid].jParent;
}

const char* GameObjectTemplateManager::GetTemplateName(unsigned int templateid)
{
    cJSON* jTemplate = GetTemplateJSONObject( templateid );

    cJSON* jName = cJSON_GetObjectItem( jTemplate, "Name" );
    return jName->valuestring;
}
