//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentRenderable.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#if MYFW_USING_WX
bool ComponentRenderable::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentRenderable ); //_VARIABLE_LIST

ComponentRenderable::ComponentRenderable(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentBase( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_pComponentTransform = 0;
}

ComponentRenderable::~ComponentRenderable()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentRenderable::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentRenderable* pThis) //_VARIABLE_LIST
{
    AddVar( pList, "Visible", ComponentVariableType::Bool, MyOffsetOf( pThis, &pThis->m_Visible ), true, true, 0,
            (CVarFunc_ValueChanged)&ComponentRenderable::OnValueChanged,
            0, 0 ); //ComponentRenderable::StaticOnDrop, 0 );

    AddVarFlags( pList, "Layers", MyOffsetOf( pThis, &pThis->m_LayersThisExistsOn ), true, true, 0,
                 g_NumberOfVisibilityLayers, g_pVisibilityLayerStrings,
                 (CVarFunc_ValueChanged)&ComponentRenderable::OnValueChanged,
                 0, 0 ); //, ComponentRenderable::StaticOnDrop, 0 );
}

void ComponentRenderable::Reset()
{
    ComponentBase::Reset();

    if( m_pGameObject )
        m_pComponentTransform = m_pGameObject->GetTransform();
    else
        m_pComponentTransform = nullptr;

    m_Visible = true;
    m_LayersThisExistsOn = Layer_MainScene;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

cJSON* ComponentRenderable::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );

    //cJSON_AddNumberToObject( jComponent, "Visible", m_Visible );
    //cJSON_AddNumberToObject( jComponent, "Layers", m_LayersThisExistsOn );

    return jComponent;
}

void ComponentRenderable::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    //cJSONExt_GetBool( jsonobj, "Visible", &m_Visible );
    //cJSONExt_GetUnsignedInt( jsonobj, "Layers", &m_LayersThisExistsOn );
}

ComponentRenderable& ComponentRenderable::operator=(const ComponentRenderable& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    this->m_Visible = other.m_Visible;
    this->m_LayersThisExistsOn = other.m_LayersThisExistsOn;

    return *this;
}

//void ComponentRenderable::RegisterCallbacks()
//{
//    if( m_Enabled && m_CallbacksRegistered == false )
//    {
//        m_CallbacksRegistered = true;
//
//        //MYFW_REGISTER_COMPONENT_CALLBACK( Tick );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( Draw );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( OnTouch );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( OnButtons );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( OnKeys );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( OnFileRenamed );
//    }
//}
//
//void ComponentRenderable::UnregisterCallbacks()
//{
//    if( m_CallbacksRegistered == true )
//    {
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );
//
//        m_CallbacksRegistered = false;
//    }
//}

bool ComponentRenderable::IsVisible()
{
    if( IsEnabled() == false )
        return false;

    MyAssert( m_pGameObject == nullptr || m_pGameObject->IsEnabled() );

    return m_Visible;
}

//void ComponentRenderable::OnGameObjectEnabled()
//{
//    ComponentBase::OnGameObjectEnabled();
//}
//
//void ComponentRenderable::OnGameObjectDisabled()
//{
//    ComponentBase::OnGameObjectDisabled();
//}

bool ComponentRenderable::SetEnabled(bool enableComponent)
{
    if( ComponentBase::SetEnabled( enableComponent ) == false )
        return false;

    if( enableComponent == false )
    {
        RemoveFromRenderGraph();
    }
    else
    {
        AddToRenderGraph();
    }

    return true;
}

void ComponentRenderable::Draw(MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride, int drawcount)
{
}

#if MYFW_EDITOR
#if MYFW_USING_WX
void ComponentRenderable::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentRenderable::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Renderable", ObjectListIcon_Component );
}

void ComponentRenderable::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentRenderable::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Renderable", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        if( addcomponentvariables )
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        //g_pPanelWatch->AddBool( "Visible", &m_Visible, 0, 1 );
        //g_pPanelWatch->AddUnsignedInt( "Layers", &m_LayersThisExistsOn, 0, 65535 );
    }
}
#endif //MYFW_USING_WX

//void* ComponentRenderable::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
//{
//    void* oldvalue = 0;
//    return oldvalue;
//}

void* ComponentRenderable::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( finishedChanging )
    {
        PushChangesToRenderGraphObjects();
    }

    return oldpointer;
}
#endif //MYFW_EDITOR
