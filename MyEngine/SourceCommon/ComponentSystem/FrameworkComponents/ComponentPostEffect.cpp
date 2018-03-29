//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentPostEffect::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentPostEffect ); //_VARIABLE_LIST

ComponentPostEffect::ComponentPostEffect()
: ComponentData()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pFullScreenQuad = 0;
    m_pMaterial = 0;
}

ComponentPostEffect::~ComponentPostEffect()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pMaterial );
}

void ComponentPostEffect::RegisterVariables(CPPListHead* pList, ComponentPostEffect* pThis) //_VARIABLE_LIST
{
    AddVar( pList, "Enabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_Enabled ), true, true, 0,
            (CVarFunc_ValueChanged)&ComponentPostEffect::OnValueChanged,
            0, 0 ); //ComponentPostEffect::StaticOnDrop, 0 );
}

cJSON* ComponentPostEffect::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* component = ComponentData::ExportAsJSONObject( savesceneid, saveid );

    if( m_pMaterial )
        cJSON_AddStringToObject( component, "Material", m_pMaterial->GetMaterialDescription() );

    return component;
}

void ComponentPostEffect::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    ComponentData::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* materialstringobj = cJSON_GetObjectItem( jsonobj, "Material" );
    if( materialstringobj )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( materialstringobj->valuestring );
        if( pMaterial )
        {
            SetMaterial( pMaterial );
            pMaterial->Release();
        }
    }
}

void ComponentPostEffect::Reset()
{
    ComponentData::Reset();

    // free old quad and material if needed.
    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pMaterial );

    m_pFullScreenQuad = MyNew MySprite( false );
    m_pFullScreenQuad->Create( 2, 2, 0, 1, 1, 0, Justify_Center, false );
    m_pMaterial = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

ComponentPostEffect& ComponentPostEffect::operator=(const ComponentPostEffect& other)
{
    MyAssert( &other != this );

    ComponentData::operator=( other );

    m_pFullScreenQuad = other.m_pFullScreenQuad;
    if( m_pFullScreenQuad )
        m_pFullScreenQuad->AddRef();

    m_pMaterial = other.m_pMaterial;
    if( m_pMaterial )
        m_pMaterial->AddRef();

    return *this;
}

void ComponentPostEffect::SetMaterial(MaterialDefinition* pMaterial)
{
    pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
}

void ComponentPostEffect::Render(FBODefinition* pFBO)
{
    MyAssert( m_pFullScreenQuad );
    MyAssert( m_pMaterial );

    if( m_pFullScreenQuad == 0 || m_pMaterial == 0 )
        return;

    m_pMaterial->SetTextureColor( pFBO->GetColorTexture( 0 ) );

    m_pFullScreenQuad->SetMaterial( m_pMaterial );
    m_pFullScreenQuad->Create( 2, 2, 0, (float)pFBO->GetWidth()/pFBO->GetTextureWidth(), (float)pFBO->GetHeight()/pFBO->GetTextureHeight(), 0, Justify_Center, false );

    if( m_pFullScreenQuad->Setup( 0, 0 ) )
    {
        Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
        pShader->ProgramDepthmap( pFBO->GetDepthTexture() );

        m_pFullScreenQuad->DrawNoSetup();
        m_pFullScreenQuad->DeactivateShader();
    }
}

#if MYFW_EDITOR
ComponentVariable* ComponentPostEffect::GetComponentVariableForMaterial(int submeshindex)
{
    return 0; //FindComponentVariableByLabel( &m_ComponentVariableList_ComponentPostEffect, "Material" );
}

#if MYFW_USING_WX
void ComponentPostEffect::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    g_pPanelObjectList->AddObject( this, ComponentPostEffect::StaticOnLeftClick, ComponentData::StaticOnRightClick, gameobjectid, "Post Effect", ObjectListIcon_Component );
}

void ComponentPostEffect::OnLeftClick(unsigned int count, bool clear)
{
    ComponentData::OnLeftClick( count, clear );
}

void ComponentPostEffect::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Post Effect", this, ComponentData::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentData::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        const char* desc = "no material";
        if( m_pMaterial && m_pMaterial->GetFile() )
            desc = m_pMaterial->GetMaterialShortDescription();
        g_pPanelWatch->AddPointerWithDescription( "Material", 0, desc, this, ComponentPostEffect::StaticOnDropMaterial );
    }
}

void ComponentPostEffect::OnDropMaterial(int controlid, int x, int y)
{
    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pDropItem->m_Value;
        MyAssert( pMaterial );

        SetMaterial( pMaterial );

        // update the panel so new Material name shows up.
        if( pMaterial->GetFile() )
            g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.GetControlID() )->m_Description = pMaterial->GetMaterialShortDescription();
    }
}
#endif //MYFW_USING_WX

void* ComponentPostEffect::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    return oldpointer;
}
#endif //MYFW_EDITOR
