//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentSprite::ComponentSprite()
: ComponentRenderable()
{
    m_BaseType = BaseComponentType_Renderable;

    m_pSprite = 0;
}

ComponentSprite::ComponentSprite(GameObject* owner)
: ComponentRenderable( owner )
{
    m_BaseType = BaseComponentType_Renderable;
}

ComponentSprite::~ComponentSprite()
{
    SAFE_RELEASE( m_pSprite );
}

#if MYFW_USING_WX
void ComponentSprite::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentSprite::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "Sprite" );
}

void ComponentSprite::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->AddUnsignedChar( "r", &m_Tint.r, 0, 255 );
    g_pPanelWatch->AddUnsignedChar( "g", &m_Tint.g, 0, 255 );
    g_pPanelWatch->AddUnsignedChar( "b", &m_Tint.b, 0, 255 );
    g_pPanelWatch->AddUnsignedChar( "a", &m_Tint.a, 0, 255 );
    g_pPanelWatch->AddFloat( "width",  &m_Size.x, 0, 2 );
    g_pPanelWatch->AddFloat( "height", &m_Size.y, 0, 2 );

    const char* desc = "no shader";
    assert( m_pSprite );
    if( m_pSprite->m_pShaderGroup )
        desc = m_pSprite->m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFilename;
    g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, this, ComponentSprite::StaticOnDropShaderGroup );
}

void ComponentSprite::OnDropShaderGroup()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)g_DragAndDropStruct.m_Value;
        assert( pShaderGroup );
        assert( m_pSprite );

        m_pSprite->SetShader( pShaderGroup );
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentSprite::ExportAsJSONObject()
{
    cJSON* component = ComponentRenderable::ExportAsJSONObject();

    cJSONExt_AddUnsignedCharArrayToObject( component, "Tint", &m_Tint.r, 4 );
    cJSONExt_AddFloatArrayToObject( component, "Size", &m_Size.x, 2 );
    cJSON_AddStringToObject( component, "Shader", m_pSprite->m_pShaderGroup->GetName() );

    return component;
}

void ComponentSprite::ImportFromJSONObject(cJSON* jsonobj)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj );

    cJSONExt_GetUnsignedCharArray( jsonobj, "Tint", &m_Tint.r, 4 );
    cJSONExt_GetFloatArray( jsonobj, "Size", &m_Size.x, 2 );
    
    cJSON* shaderstringobj = cJSON_GetObjectItem( jsonobj, "Shader" );
    ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByName( shaderstringobj->valuestring );
    m_pSprite->SetShader( pShaderGroup );
}

void ComponentSprite::Reset()
{
    ComponentRenderable::Reset();

    if( m_pSprite == 0 )
        m_pSprite = MyNew MySprite();

    m_Size.Set( 1.0f, 1.0f );
    m_Tint.Set( 255,255,255,255 );
}

void ComponentSprite::SetShader(ShaderGroup* pShader)
{
    ComponentRenderable::SetShader( pShader );

    m_pSprite->SetShader( pShader );
}

void ComponentSprite::Draw(MyMatrix* pMatViewProj)
{
    ComponentRenderable::Draw(pMatViewProj);

    m_pSprite->SetPosition( &m_pComponentTransform->m_Transform );
    m_pSprite->SetTint( m_Tint );
    m_pSprite->Create( "ComponentSprite", m_Size.x, m_Size.y, 0, 1, 0, 1, Justify_Center, false );
    m_pSprite->Draw( pMatViewProj );
}
