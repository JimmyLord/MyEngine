//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

ComponentBase::ComponentBase()
: m_SceneIDLoadedFrom( 0 )
, m_BaseType( BaseComponentType_None )
, m_pGameObject( 0 )
, m_Type(-1)
, m_ID(0)
, m_Enabled( true )
{
    ClassnameSanityCheck();

#if MYFW_USING_WX
    m_ControlID_ComponentTitleLabel = -1;
    m_pPanelWatchBlockVisible = 0;
#endif

    m_CallbacksRegistered = false;
}

ComponentBase::~ComponentBase()
{
    // Components must be disabled before being deleted, so they unregister their callbacks.
    MyAssert( m_Enabled == false );
    MyAssert( m_CallbacksRegistered == false );

    // if it's in a list, remove it.
    if( this->Prev != 0 )
        Remove();

//    ClearAllVariables_Base( GetComponentVariableList() );
}

void ComponentBase::Reset()
{
#if MYFW_USING_WX
    m_ControlID_ComponentTitleLabel = -1;
    m_pPanelWatchBlockVisible = 0;
#endif
}

void ComponentBase::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentBase>( "ComponentBase" )
            //.addData( "localmatrix", &ComponentBase::m_LocalTransform )
            
            .addFunction( "SetEnabled", &ComponentBase::SetEnabled )
            .addFunction( "IsEnabled", &ComponentBase::IsEnabled )
            
            .addFunction( "SetSceneID", &ComponentBase::SetSceneID )
            .addFunction( "GetSceneID", &ComponentBase::GetSceneID )
            
            .addFunction( "SetID", &ComponentBase::SetID )
            .addFunction( "GetID", &ComponentBase::GetID )
        .endClass();
}

void ComponentBase::SetEnabled(bool enabled)
{
    if( m_Enabled == enabled )
        return;

    m_Enabled = enabled;

    if( enabled )
        RegisterCallbacks();
    else
        UnregisterCallbacks();
}

//CPPListHead ComponentBase::m_ComponentVariableList;

void ComponentBase::ClearAllVariables_Base(CPPListHead* pComponentVariableList)
{
    while( CPPListNode* pNode = pComponentVariableList->GetHead() )
    {
        ComponentVariable* pVariable = (ComponentVariable*)pNode;
        pVariable->Remove();
        delete pVariable;
    }
}

void ComponentBase::AddVariable_Base(CPPListHead* pComponentVariableList, const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, ComponentVariableCallbackValueChanged pOnValueChangedCallBackFunc, ComponentVariableCallbackDropTarget pOnDropCallBackFunc, ComponentVariableCallback pOnButtonPressedCallBackFunc)
{
    ComponentVariable* pVariable = MyNew ComponentVariable( label, type, offset, saveload, displayinwatch,
        watchlabel, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc, 0, 0, 0 );
    pComponentVariableList->AddTail( pVariable );
}

void ComponentBase::AddVariablePointer_Base(CPPListHead* pComponentVariableList, const char* label, bool saveload, bool displayinwatch, const char* watchlabel, ComponentVariableCallbackValueChanged pOnValueChangedCallBackFunc, ComponentVariableCallbackDropTarget pOnDropCallBackFunc, ComponentVariableCallback pOnButtonPressedCallBackFunc, ComponentVariableCallbackPointer pGetPointerValueCallBackFunc, ComponentVariableCallbackGetPointerDesc pGetPointerDescCallBackFunc, ComponentVariableCallbackSetPointerDesc pSetPointerDescCallBackFunc)
{
    ComponentVariable* pVariable = MyNew ComponentVariable( label, ComponentVariableType_PointerIndirect, -1, saveload, displayinwatch,
        watchlabel, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc,
        pGetPointerValueCallBackFunc, pGetPointerDescCallBackFunc, pSetPointerDescCallBackFunc );
    pComponentVariableList->AddTail( pVariable );
}

void ComponentBase::FillPropertiesWindowWithVariables()
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_DisplayInWatch == false )
            continue;

        //if( pVar->m_Offset != -1 )
        {
            switch( pVar->m_Type )
            {
            case ComponentVariableType_ColorByte:
                pVar->m_ControlID = g_pPanelWatch->AddColorByte( pVar->m_WatchLabel, (ColorByte*)((char*)this + pVar->m_Offset), 0.0f, 0.0f, this, ComponentBase::StaticOnValueChangedVariable );
                break;

            case ComponentVariableType_Vector2:
                pVar->m_ControlID = g_pPanelWatch->AddVector2( pVar->m_WatchLabel, (Vector2*)((char*)this + pVar->m_Offset), 0.0f, 0.0f, this, ComponentBase::StaticOnValueChangedVariable );
                break;

            case ComponentVariableType_Vector3:
                pVar->m_ControlID = g_pPanelWatch->AddVector3( pVar->m_WatchLabel, (Vector3*)((char*)this + pVar->m_Offset), 0.0f, 0.0f, this, ComponentBase::StaticOnValueChangedVariable );
                break;

            case ComponentVariableType_GameObjectPtr:
                MyAssert( false );
                break;

            case ComponentVariableType_ComponentPtr:
                {
                    ComponentTransform* pTransformComponent = *(ComponentTransform**)((char*)this + pVar->m_Offset);

                    const char* desc = "none";
                    if( pTransformComponent )
                    {
                        desc = pTransformComponent->m_pGameObject->GetName();
                    }

                    pVar->m_ControlID = g_pPanelWatch->AddPointerWithDescription( pVar->m_WatchLabel, pTransformComponent, desc, this, ComponentBase::StaticOnDropVariable, ComponentBase::StaticOnValueChangedVariable );
                }
                break;

            case ComponentVariableType_FilePtr:
                {
                    MyFileObject* pFile = *(MyFileObject**)((char*)this + pVar->m_Offset);

                    const char* desc = "none";
                    if( pFile )
                    {
                        desc = pFile->m_FullPath;
                    }

                    pVar->m_ControlID = g_pPanelWatch->AddPointerWithDescription( pVar->m_WatchLabel, pFile, desc, this, ComponentBase::StaticOnDropVariable, ComponentBase::StaticOnValueChangedVariable );
                }
                break;

            case ComponentVariableType_PointerIndirect:
                {
                    void* pPtr = pVar->m_pGetPointerValueCallBackFunc( this, pVar );
                    const char* pDesc = pVar->m_pGetPointerDescCallBackFunc( this, pVar );
                    pVar->m_ControlID = g_pPanelWatch->AddPointerWithDescription( pVar->m_WatchLabel, pPtr, pDesc, this, ComponentBase::StaticOnDropVariable, ComponentBase::StaticOnValueChangedVariable );
                }
                break;

            case ComponentVariableType_NumTypes:
            default:
                MyAssert( false );
                break;
            }
        }
    }
}

void ComponentBase::ExportVariablesToJSON(cJSON* jComponent)
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_SaveLoad == false )
            continue;

        //if( pVar->m_Offset != -1 )
        {
            switch( pVar->m_Type )
            {
            case ComponentVariableType_ComponentPtr:
            case ComponentVariableType_FilePtr:
                MyAssert( false );
                break;

            case ComponentVariableType_ColorByte:
                cJSONExt_AddUnsignedCharArrayToObject( jComponent, pVar->m_Label, (unsigned char*)((char*)this + pVar->m_Offset), 4 );
                break;

            case ComponentVariableType_Vector2:
                cJSONExt_AddFloatArrayToObject( jComponent, pVar->m_Label, (float*)((char*)this + pVar->m_Offset), 2 );
                break;

            case ComponentVariableType_Vector3:
                cJSONExt_AddFloatArrayToObject( jComponent, pVar->m_Label, (float*)((char*)this + pVar->m_Offset), 3 );
                break;

            case ComponentVariableType_GameObjectPtr:
                {
                    GameObject* pParentGameObject = *(GameObject**)((char*)this + pVar->m_Offset);
                    if( pParentGameObject )
                    {
                        cJSON_AddNumberToObject( jComponent, pVar->m_Label, pParentGameObject->GetID() );
                    }
                }
                break;

            case ComponentVariableType_PointerIndirect:
                MyAssert( pVar->m_pGetPointerDescCallBackFunc );
                if( pVar->m_pGetPointerDescCallBackFunc )
                    cJSON_AddStringToObject( jComponent, pVar->m_Label, pVar->m_pGetPointerDescCallBackFunc( this, pVar ) );
                break;

            case ComponentVariableType_NumTypes:
            default:
                MyAssert( false );
                break;
            }
        }
    }
}

void ComponentBase::ImportVariablesFromJSON(cJSON* jsonobj, const char* singlelabeltoimport)
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_SaveLoad == false )
            continue;

        // if we are looking for a single label to import, check if this is it.
        if( singlelabeltoimport != 0 && strcmp( singlelabeltoimport, pVar->m_Label ) != 0 )
            continue;

        //if( pVar->m_Offset != -1 )
        {
            switch( pVar->m_Type )
            {
            case ComponentVariableType_ComponentPtr:
            case ComponentVariableType_FilePtr:
                MyAssert( false );
                break;

            case ComponentVariableType_ColorByte:
                cJSONExt_GetUnsignedCharArray( jsonobj, pVar->m_Label, (unsigned char*)((char*)this + pVar->m_Offset), 4 );
                break;

            case ComponentVariableType_Vector2:
                cJSONExt_GetFloatArray( jsonobj, pVar->m_Label, (float*)((char*)this + pVar->m_Offset), 2 );
                break;

            case ComponentVariableType_Vector3:
                cJSONExt_GetFloatArray( jsonobj, pVar->m_Label, (float*)((char*)this + pVar->m_Offset), 3 );
                break;

            case ComponentVariableType_GameObjectPtr:
                {
                    unsigned int parentid = 0;
                    cJSONExt_GetUnsignedInt( jsonobj, pVar->m_Label, &parentid );
                    if( parentid != 0 )
                    {
                        GameObject* pParentGameObject = g_pComponentSystemManager->FindGameObjectByID( m_SceneIDLoadedFrom, parentid );
                        *(GameObject**)((char*)this + pVar->m_Offset) = pParentGameObject;
                    }
                }
                break;

            case ComponentVariableType_PointerIndirect:
                MyAssert( pVar->m_pSetPointerDescCallBackFunc );
                if( pVar->m_pSetPointerDescCallBackFunc )
                {
                    cJSON* obj = cJSON_GetObjectItem( jsonobj, pVar->m_Label );
                    MyAssert( obj );
                    if( obj )
                        pVar->m_pSetPointerDescCallBackFunc( this, pVar, obj->valuestring );
                }
                break;

            case ComponentVariableType_NumTypes:
            default:
                MyAssert( false );
                break;
            }
        }
    }
}

int ComponentBase::FindVariablesControlIDByLabel(const char* label)
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( strcmp( pVar->m_Label, label ) == 0 )
            return pVar->m_ControlID;
    }

    return -1;
}

#if MYFW_USING_WX
void ComponentBase::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentBase::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Unknown component" );
    g_pPanelObjectList->SetDragAndDropFunctions( id, ComponentBase::StaticOnDrag, ComponentBase::StaticOnDrop );
}

void ComponentBase::OnValueChangedVariable(int controlid, bool finishedchanging, double oldvalue)
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_ControlID == controlid ||
            (pVar->m_Type == ComponentVariableType_Vector3 && (pVar->m_ControlID+1 == controlid || pVar->m_ControlID+2 == controlid) ) ||
            (pVar->m_Type == ComponentVariableType_Vector2 && (pVar->m_ControlID+1 == controlid) )
          )
        {
            void* oldobjectvalue = pVar->m_pOnValueChangedCallbackFunc( this, pVar, finishedchanging, oldvalue );

            // find children of this gameobject and change their values as well, if their value matches the old value.
            for( CPPListNode* pCompNode = g_pComponentSystemManager->m_GameObjects.GetHead(); pCompNode; pCompNode = pCompNode->GetNext() )
            {
                GameObject* pGameObject = (GameObject*)pCompNode;

                if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
                {
                    // Found a game object, now find the matching component on it.
                    for( unsigned int i=0; i<pGameObject->m_Components.Count()+1; i++ )
                    {
                        ComponentBase* pComponent;

                        if( i == 0 )
                            pComponent = pGameObject->m_pComponentTransform;
                        else
                            pComponent = pGameObject->m_Components[i-1];

                        const char* pThisCompClassName = GetClassname();
                        const char* pOtherCompClassName = pComponent->GetClassname();

                        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
                        {
                            // TODO: this will fail if multiple of the same component are on an object.

                            // Found the matching component, now compare the variable.
                            switch( pVar->m_Type )
                            {
                            case ComponentVariableType_GameObjectPtr:
                            case ComponentVariableType_FilePtr:
                                MyAssert( false );
                                break;

                            case ComponentVariableType_ColorByte:
                                // TODO: changing color in panel watch doesn't call OnValueChanged
                                MyAssert( false );
                                break;

                            case ComponentVariableType_Vector2:
                            case ComponentVariableType_Vector3:
                                {
                                    // figure out which component of a multi-component control(e.g. vector3) this is.
                                    int controlcomponent = controlid - pVar->m_ControlID;

                                    int offset = pVar->m_Offset + controlcomponent*4;

                                    if( *(float*)((char*)pComponent + offset) == oldvalue )
                                    {
                                        *(float*)((char*)pComponent + offset) = *(float*)((char*)this + offset);
                                        pVar->m_pOnValueChangedCallbackFunc( pComponent, pVar, finishedchanging, oldvalue );
                                    }
                                }
                                break;

                            case ComponentVariableType_ComponentPtr:
                                {
                                    int offset = pVar->m_Offset;

                                    if( *(ComponentBase**)((char*)pComponent + pVar->m_Offset) == oldobjectvalue )
                                    {
                                        void* oldobjectvalue2 = pVar->m_pOnValueChangedCallbackFunc( pComponent, pVar, finishedchanging, oldvalue );
                                        MyAssert( oldobjectvalue2 == oldobjectvalue );
                                    }                                
                                }
                                break;

                            case ComponentVariableType_PointerIndirect:
                                // TODO: support typing a new description for this type
                                MyAssert( false );
                                break;

                            case ComponentVariableType_NumTypes:
                            default:
                                MyAssert( false );
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void ComponentBase::OnDropVariable(int controlid, wxCoord x, wxCoord y)
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_ControlID == controlid ||
            (pVar->m_Type == ComponentVariableType_Vector3 && (pVar->m_ControlID+1 == controlid || pVar->m_ControlID+2 == controlid) )
          )
        {
            // OnDropCallback will grab the new value from g_DragAndDropStruct
            if( pVar->m_pOnDropCallbackFunc == 0 )
                return;

            void* oldvalue = pVar->m_pOnDropCallbackFunc( this, pVar, x, y );

            // TODO: propagate change down to child objects...
            for( CPPListNode* pCompNode = g_pComponentSystemManager->m_GameObjects.GetHead(); pCompNode; pCompNode = pCompNode->GetNext() )
            {
                GameObject* pGameObject = (GameObject*)pCompNode;

                if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
                {
                    // Found a game object, now find the matching component on it.
                    for( unsigned int i=0; i<pGameObject->m_Components.Count()+1; i++ )
                    {
                        ComponentBase* pComponent;

                        if( i == 0 )
                            pComponent = pGameObject->m_pComponentTransform;
                        else
                            pComponent = pGameObject->m_Components[i-1];

                        const char* pThisCompClassName = GetClassname();
                        const char* pOtherCompClassName = pComponent->GetClassname();

                        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
                        {
                            // TODO: this will fail if multiple of the same component are on an object.

                            // Found the matching component, now compare the variable.
                            switch( pVar->m_Type )
                            {
                            case ComponentVariableType_GameObjectPtr:
                                MyAssert( false );
                                break;

                            case ComponentVariableType_ColorByte:
                            case ComponentVariableType_Vector2:
                            case ComponentVariableType_Vector3:
                                MyAssert( false ); // not drag/dropping these types ATM.
                                break;

                            case ComponentVariableType_ComponentPtr:
                            case ComponentVariableType_FilePtr:
                                {
                                    int offset = pVar->m_Offset;

                                    if( *(ComponentBase**)((char*)pComponent + offset) == oldvalue )
                                    {
                                        // OnDropCallback will grab the new value from g_DragAndDropStruct
                                        void* oldvalue2 = pVar->m_pOnDropCallbackFunc( pComponent, pVar, x, y );
                                        MyAssert( oldvalue2 == oldvalue );
                                    }
                                }
                                break;

                            case ComponentVariableType_PointerIndirect:
                                {
                                    int offset = pVar->m_Offset;

                                    if( pVar->m_pGetPointerValueCallBackFunc( pComponent, pVar ) == oldvalue )
                                    {
                                        // OnDropCallback will grab the new value from g_DragAndDropStruct
                                        void* oldvalue2 = pVar->m_pOnDropCallbackFunc( pComponent, pVar, x, y );
                                        MyAssert( oldvalue2 == oldvalue );
                                    }
                                }
                                break;

                            case ComponentVariableType_NumTypes:
                            default:
                                MyAssert( false );
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void ComponentBase::OnComponentTitleLabelClicked(int id, bool finishedchanging)
{
    if( id != -1 && id == m_ControlID_ComponentTitleLabel )
    {
        if( m_pPanelWatchBlockVisible )
        {
            *m_pPanelWatchBlockVisible = !(*m_pPanelWatchBlockVisible);
            g_pPanelWatch->m_NeedsRefresh = true;
        }
    }
}

void ComponentBase::OnLeftClick(unsigned int count, bool clear)
{
    // select this Component in the editor window.
    g_pEngineCore->m_pEditorState->m_pSelectedComponents.push_back( this );

    if( clear )
        g_pPanelWatch->ClearAllVariables();

    FillPropertiesWindow( clear );
}

void ComponentBase::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    AppendItemsToRightClickMenu( &menu );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentBase::AppendItemsToRightClickMenu(wxMenu* pMenu)
{
    pMenu->Append( 1000, "Delete Component" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentBase::OnPopupClick );
}

void ComponentBase::OnPopupClick(wxEvent &evt)
{
    //ComponentBase* pComponent = (ComponentBase*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    int id = evt.GetId();
    if( id == 1000 )
    {
        EditorState* pEditorState = g_pEngineCore->m_pEditorState;

        // deselect all "main" transform components.
        for( unsigned int i=0; i<pEditorState->m_pSelectedComponents.size(); i++ )
        {
            ComponentBase* pSelComp = pEditorState->m_pSelectedComponents[i];
            if( pSelComp->m_pGameObject && pSelComp == pSelComp->m_pGameObject->m_pComponentTransform )
            {
                pEditorState->m_pSelectedComponents[i] = pEditorState->m_pSelectedComponents.back();
                pEditorState->m_pSelectedComponents.pop_back();
                i--;
            }
        }

        // if anything is still selected, delete it/them.
        if( pEditorState->m_pSelectedComponents.size() > 0 )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteComponents( pEditorState->m_pSelectedComponents ) );
        }

        //// if the object isn't selected, delete just the one object, otherwise delete all selected objects.
        //if( pEditorState->IsComponentSelected( pComponent ) )
        //{
        //    g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteComponents( pEditorState->m_pSelectedComponents ) );
        //}
        //else
        //{
        //    // create a temp vector to pass into command.
        //    std::vector<ComponentBase*> components;
        //    components.push_back( pComponent );
        //    g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteComponents( components ) );
        //}
    }
}

void ComponentBase::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_ComponentPointer;
    g_DragAndDropStruct.m_Value = this;
}

void ComponentBase::OnDrop(int controlid, wxCoord x, wxCoord y)
{
}
#endif //MYFW_USING_WX

cJSON* ComponentBase::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = cJSON_CreateObject();

    //cJSON_AddNumberToObject( component, "BaseType", m_BaseType );

    if( savesceneid )
        cJSON_AddNumberToObject( component, "SceneID", m_SceneIDLoadedFrom );

    if( m_Type != -1 )
    {
        const char* componenttypename = g_pComponentTypeManager->GetTypeName( m_Type );
        MyAssert( componenttypename );
        if( componenttypename )
            cJSON_AddStringToObject( component, "Type", componenttypename );
    }

    if( m_pGameObject )
        cJSON_AddNumberToObject( component, "GOID", m_pGameObject->GetID() );

    cJSON_AddNumberToObject( component, "ID", m_ID );

    return component;
}

void ComponentBase::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    cJSONExt_GetUnsignedInt( jsonobj, "ID", &m_ID );

    MyAssert( m_SceneIDLoadedFrom == 0 || m_SceneIDLoadedFrom == sceneid );
    SetSceneID( sceneid );
}

ComponentBase& ComponentBase::operator=(const ComponentBase& other)
{
    MyAssert( &other != this );

    return *this;
}

void ComponentBase::OnLoad()
{
    if( m_Enabled && m_pGameObject && m_pGameObject->IsEnabled() )
        RegisterCallbacks();
    else
        UnregisterCallbacks();
}

void ComponentBase::OnGameObjectEnabled()
{
    if( m_Enabled )
        RegisterCallbacks();
}

void ComponentBase::OnGameObjectDisabled()
{
    UnregisterCallbacks();
}
