//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentTemplate::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentTemplate ); //_VARIABLE_LIST

ComponentTemplate::ComponentTemplate()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_SampleVector3.Set( 0, 0, 0 );
}

ComponentTemplate::~ComponentTemplate()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentTemplate::RegisterVariables(CPPListHead* pList, ComponentTemplate* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
    MyAssert( offsetof( ComponentTemplate, m_SampleVector3 ) == MyOffsetOf( pThis, &pThis->m_SampleVector3 ) );

    AddVariable( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, ComponentTemplate::StaticOnValueChanged, ComponentTemplate::StaticOnDrop, 0 );
}

void ComponentTemplate::Reset()
{
    ComponentBase::Reset();

    m_SampleVector3.Set( 0, 0, 0 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentTemplate::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentTemplate::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Template" );
}

void ComponentTemplate::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentTemplate::FillPropertiesWindow(bool clear, bool addcomponentvariables)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Template", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentTemplate::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        (ComponentBase*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        (GameObject*)g_DragAndDropStruct.m_Value;
    }

    return oldvalue;
}

void* ComponentTemplate::OnValueChanged(ComponentVariable* pVar, bool finishedchanging)
{
    void* oldvalue = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_SampleVector3 ) )
    {
        MyAssert( pVar->m_ControlID != -1 );
    }

    return oldvalue;
}
#endif //MYFW_USING_WX

cJSON* ComponentTemplate::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );

    ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST

    return jComponent;
}

void ComponentTemplate::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    ImportVariablesFromJSON( jsonobj ); //_VARIABLE_LIST
}

void ComponentTemplate::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( OnFileRenamed );
    }
}

void ComponentTemplate::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

ComponentTemplate& ComponentTemplate::operator=(const ComponentTemplate& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_SampleVector3 = other.m_SampleVector3;

    return *this;
}