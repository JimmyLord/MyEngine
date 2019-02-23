//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentPostEffect.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentPostEffect ); //_VARIABLE_LIST

ComponentPostEffect::ComponentPostEffect()
: ComponentData()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pFullScreenQuad = nullptr;
    m_pMaterial = nullptr;
}

ComponentPostEffect::~ComponentPostEffect()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pMaterial );
}

void ComponentPostEffect::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentPostEffect* pThis) //_VARIABLE_LIST
{
    //AddVar( pList, "Enabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_Enabled ), true, true, nullptr,
    //        (CVarFunc_ValueChanged)&ComponentPostEffect::OnValueChanged,
    //        nullptr, nullptr );
    AddVar( pList, "Material", ComponentVariableType_MaterialPtr,
            MyOffsetOf( pThis, &pThis->m_pMaterial ), false, true, 
            nullptr, (CVarFunc_ValueChanged)&ComponentPostEffect::OnValueChanged, (CVarFunc_DropTarget)&ComponentPostEffect::OnDropMaterial, nullptr );
}

cJSON* ComponentPostEffect::ExportAsJSONObject(bool saveSceneID, bool saveID)
{
    cJSON* component = ComponentData::ExportAsJSONObject( saveSceneID, saveID );

    if( m_pMaterial )
        cJSON_AddStringToObject( component, "Material", m_pMaterial->GetMaterialDescription() );

    return component;
}

void ComponentPostEffect::ImportFromJSONObject(cJSON* jObject, SceneID sceneID)
{
    ComponentData::ImportFromJSONObject( jObject, sceneID );

    cJSON* materialstringobj = cJSON_GetObjectItem( jObject, "Material" );
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

    // Free old quad and material if needed.
    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pMaterial );

    m_pFullScreenQuad = MyNew MySprite( false );
    m_pFullScreenQuad->Create( 2, 2, 0, 1, 1, 0, Justify_Center, false );
    m_pMaterial = nullptr;

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

    if( m_pFullScreenQuad == nullptr || m_pMaterial == nullptr )
        return;

    m_pMaterial->SetTextureColor( pFBO->GetColorTexture( 0 ) );

    m_pFullScreenQuad->SetMaterial( m_pMaterial );
    m_pFullScreenQuad->Create( 2, 2, 0, (float)pFBO->GetWidth()/pFBO->GetTextureWidth(), (float)pFBO->GetHeight()/pFBO->GetTextureHeight(), 0, Justify_Center, false );

    if( m_pFullScreenQuad->Setup( nullptr, nullptr, nullptr ) )
    {
        Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
        pShader->ProgramDepthmap( pFBO->GetDepthTexture() );

        m_pFullScreenQuad->DrawNoSetup();
        m_pFullScreenQuad->DeactivateShader();
    }
}

#if MYFW_EDITOR
ComponentVariable* ComponentPostEffect::GetComponentVariableForMaterial(int submeshIndex)
{
    return nullptr; //FindComponentVariableByLabel( &m_ComponentVariableList_ComponentPostEffect, "Material" );
}

void* ComponentPostEffect::OnDropMaterial(ComponentVariable* pVar, int x, int y)
{
    void* oldPointer = nullptr;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pDropItem->m_Value;

        oldPointer = GetMaterial();
        SetMaterial( pMaterial );
        //g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeMaterialOnMesh( this, pVar, materialthatchanged, pMaterial ) );
    }

    return oldPointer;
}

void* ComponentPostEffect::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = nullptr;

    return oldPointer;
}
#endif //MYFW_EDITOR
