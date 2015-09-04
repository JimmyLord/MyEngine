//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentSprite::m_PanelWatchBlockVisible = true;
#endif

ComponentSprite::ComponentSprite()
: ComponentRenderable()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_pSprite = 0;
}

ComponentSprite::~ComponentSprite()
{
    SAFE_RELEASE( m_pSprite );
}

#if MYFW_USING_WX
void ComponentSprite::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentSprite::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Sprite" );
}

void ComponentSprite::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentSprite::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Sprite", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        g_pPanelWatch->AddUnsignedChar( "r", &m_Tint.r, 0, 255 );
        g_pPanelWatch->AddUnsignedChar( "g", &m_Tint.g, 0, 255 );
        g_pPanelWatch->AddUnsignedChar( "b", &m_Tint.b, 0, 255 );
        g_pPanelWatch->AddUnsignedChar( "a", &m_Tint.a, 0, 255 );
        g_pPanelWatch->AddFloat( "width",  &m_Size.x, 0, 0 );
        g_pPanelWatch->AddFloat( "height", &m_Size.y, 0, 0 );

        const char* desc = "no material";
        MyAssert( m_pSprite );
        MaterialDefinition* pMaterial = m_pSprite->GetMaterial();
        if( pMaterial && pMaterial->m_pFile )
            desc = pMaterial->m_pFile->m_FullPath;
        g_pPanelWatch->AddPointerWithDescription( "Material", 0, desc, this, ComponentSprite::StaticOnDropMaterial );
    }
}

void ComponentSprite::OnDropMaterial(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
        MyAssert( pMaterial );
        MyAssert( m_pSprite );

        m_pSprite->SetMaterial( pMaterial );
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentSprite::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = ComponentRenderable::ExportAsJSONObject( savesceneid );

    cJSONExt_AddUnsignedCharArrayToObject( component, "Tint", &m_Tint.r, 4 );
    cJSONExt_AddFloatArrayToObject( component, "Size", &m_Size.x, 2 );
    if( m_pSprite->GetMaterial() && m_pSprite->GetMaterial()->m_pFile )
        cJSON_AddStringToObject( component, "Material", m_pSprite->GetMaterial()->m_pFile->m_FullPath );

    return component;
}

void ComponentSprite::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetUnsignedCharArray( jsonobj, "Tint", &m_Tint.r, 4 );
    cJSONExt_GetFloatArray( jsonobj, "Size", &m_Size.x, 2 );
    
    cJSON* materialobj = cJSON_GetObjectItem( jsonobj, "Material" );
    if( materialobj )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( materialobj->valuestring );
        if( pMaterial )
            m_pSprite->SetMaterial( pMaterial );
        pMaterial->Release();
    }
}

void ComponentSprite::Reset()
{
    ComponentRenderable::Reset();

    if( m_pSprite == 0 )
        m_pSprite = MyNew MySprite( true );

    m_Size.Set( 1.0f, 1.0f );
    m_Tint.Set( 255,255,255,255 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

ComponentSprite* CastAs_ComponentSprite(ComponentBase* pComponent)
{
    MyAssert( pComponent->IsA( "SpriteComponent" ) );
    return (ComponentSprite*)pComponent;
}

void ComponentSprite::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate ).addFunction( "CastAs_ComponentSprite", CastAs_ComponentSprite );

    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSprite>( "ComponentSprite" )
            //.addData( "localmatrix", &ComponentSprite::m_LocalTransform )
            
            .addFunction( "GetSprite", &ComponentSprite::GetSprite )
        .endClass();
}

ComponentSprite& ComponentSprite::operator=(const ComponentSprite& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    this->m_Tint = other.m_Tint;
    this->m_Size = other.m_Size;
    this->m_pSprite->SetMaterial( other.m_pSprite->GetMaterial() );

    return *this;
}

void ComponentSprite::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    ComponentRenderable::SetMaterial( pMaterial, 0 );

    m_pSprite->SetMaterial( pMaterial );
}

void ComponentSprite::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, drawcount );

    m_pSprite->SetPosition( &m_pComponentTransform->m_Transform );
    m_pSprite->SetTint( m_Tint );
    m_pSprite->Create( "ComponentSprite", m_Size.x, m_Size.y, 0, 1, 0, 1, Justify_Center, false );
    m_pSprite->Draw( pMatViewProj );
}
