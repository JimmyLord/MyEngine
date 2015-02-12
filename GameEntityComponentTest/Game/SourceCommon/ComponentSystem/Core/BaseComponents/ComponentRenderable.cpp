//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

bool ComponentRenderable::m_PanelWatchBlockVisible = true;

ComponentRenderable::ComponentRenderable()
: ComponentBase()
{
    m_BaseType = BaseComponentType_Renderable;
}

ComponentRenderable::~ComponentRenderable()
{
}

void ComponentRenderable::Reset()
{
    ComponentBase::Reset();

    assert( m_pGameObject );

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;

    m_Visible = true;
    m_LayersThisExistsOn = Layer_MainScene;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentRenderable::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentRenderable::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Renderable" );
}

void ComponentRenderable::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentRenderable::FillPropertiesWindow(bool clear)
{
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Renderable", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentBase::FillPropertiesWindow( clear );

        g_pPanelWatch->AddUnsignedInt( "Layers", &m_LayersThisExistsOn, 0, 63 );
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentRenderable::ExportAsJSONObject()
{
    cJSON* component = ComponentBase::ExportAsJSONObject();

    cJSON_AddNumberToObject( component, "Visible", m_Visible );
    cJSON_AddNumberToObject( component, "Layers", m_LayersThisExistsOn );

    return component;
}

void ComponentRenderable::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetBool( jsonobj, "Visible", &m_Visible );
    cJSONExt_GetUnsignedInt( jsonobj, "Layers", &m_LayersThisExistsOn );
}

ComponentRenderable& ComponentRenderable::operator=(const ComponentRenderable& other)
{
    assert( &other != this );

    ComponentBase::operator=( other );

    //this->m_pShader = other.m_pShader;
    //this->m_pShader->AddRef();

    this->m_Visible = other.m_Visible;
    this->m_LayersThisExistsOn = other.m_LayersThisExistsOn;

    return *this;
}

void ComponentRenderable::SetShader(ShaderGroup* pShader)
{
    //pShader->AddRef();
    //SAFE_RELEASE( m_pShader );
    //m_pShader = pShader;
}

void ComponentRenderable::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
#if 1 //MYFW_USING_WX
    // ugh, for now... matrix will be dirty when playing with watch window in wx 
    m_pComponentTransform->UpdateMatrix();
#endif
}
