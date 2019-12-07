//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentBase.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#endif

#if MYFW_USING_IMGUI
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
#include "../SourceEditor/Editor_ImGui/ImGuiStylePrefs.h"
#endif

ComponentBase::ComponentBase(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: m_pEngineCore( pEngineCore )
, m_pComponentSystemManager( pComponentSystemManager )
, m_SceneIDLoadedFrom( SCENEID_NotSet )
, m_BaseType( BaseComponentType_None )
, m_pGameObject( 0 )
, m_Type(-1)
, m_ID(0)
, m_EnabledState( EnabledState_Enabled )
{
    ClassnameSanityCheck();

#if MYFW_EDITOR
    m_DivorcedVariables = 0;
    m_PrefabComponentID = 0;
#endif

#if MYFW_USING_WX
    m_ControlID_ComponentTitleLabel = -1;
    m_pPanelWatchBlockVisible = 0;
#endif

#if MYFW_USING_IMGUI
    //m_ComponentVariableValueWhenControlSelected;
    m_ImGuiControlIDForCurrentlySelectedVariable = -1;
    m_LinkNextUndoCommandToPrevious = false;
#endif

    m_CallbacksRegistered = false;
}

ComponentBase::~ComponentBase()
{
#if MYFW_USING_WX
    if( g_pPanelWatch->GetObjectBeingWatched() == this )
        g_pPanelWatch->ClearAllVariables();
#endif //MYFW_USING_WX

    // Components must be disabled before being deleted, so they unregister their callbacks.
    MyAssert( m_EnabledState == EnabledState_Disabled_ManuallyDisabled );
    MyAssert( m_CallbacksRegistered == false );

    // Let others know we were deleted.
    NotifyOthersThisWasDeleted();
    MyAssert( m_pOnDeleteCallbacks.GetHead() == 0 );

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

#if MYFW_USING_LUA
void ComponentBase::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentBase>( "ComponentBase" )
            //.addData( "localmatrix", &ComponentBase::m_LocalTransform )
            
            .addFunction( "IsA", &ComponentBase::IsA ) // bool ComponentBase::IsA(const char* classname)

            .addFunction( "GetTypeName", &ComponentBase::GetTypeName ) // const char* GetTypeName()

            .addFunction( "SetEnabled", &ComponentBase::SetEnabled ) // bool ComponentBase::SetEnabled(bool enabled)
            .addFunction( "IsEnabled", &ComponentBase::IsEnabled ) // bool ComponentBase::IsEnabled()
            
            .addFunction( "GetSceneID", &ComponentBase::GetSceneID ) // unsigned int ComponentBase::GetSceneID()
            .addFunction( "GetID", &ComponentBase::GetID ) // unsigned int ComponentBase::GetID()
        .endClass();
}
#endif //MYFW_USING_LUA

cJSON* ComponentBase::ExportAsJSONObject(bool saveSceneID, bool saveID)
{
    cJSON* jComponent = cJSON_CreateObject();

    //cJSON_AddNumberToObject( jComponent, "BaseType", m_BaseType );

    if( saveSceneID )
    {
        cJSON_AddNumberToObject( jComponent, "SceneID", m_SceneIDLoadedFrom );
    }

    if( m_EnabledState == EnabledState_Disabled_ManuallyDisabled )
    {
        cJSON_AddBoolToObject( jComponent, "Enabled", false );
    }

    // Transform are saved as a dedicated transforms array, so don't print the Type name for them.
    if( m_Type != -1 && m_Type != ComponentType_Transform )
    {
        const char* componenttypename = g_pComponentTypeManager->GetTypeName( m_Type );
        MyAssert( componenttypename );
        if( componenttypename )
            cJSON_AddStringToObject( jComponent, "Type", componenttypename );
    }

    if( saveID )
    {
        if( m_pGameObject )
        {
            cJSON_AddNumberToObject( jComponent, "GOID", m_pGameObject->GetID() );
        }

        cJSON_AddNumberToObject( jComponent, "ID", m_ID );
    }

#if MYFW_EDITOR
    cJSON_AddNumberToObject( jComponent, "PrefabComponentID", m_PrefabComponentID );
#endif

    // TODO: this will break if more variables are added to a component or it's parents.
    if( m_pGameObject && m_pGameObject->GetGameObjectThisInheritsFrom() != 0 && m_DivorcedVariables != 0 )
        cJSON_AddNumberToObject( jComponent, "Divorced", m_DivorcedVariables );

    ExportVariablesToJSON( jComponent, this, GetComponentVariableList(), this ); //_VARIABLE_LIST

    return jComponent;
}

void ComponentBase::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
{
    cJSONExt_GetUnsignedInt( jComponent, "ID", &m_ID );

#if MYFW_EDITOR
    cJSONExt_GetUnsignedInt( jComponent, "PrefabComponentID", &m_PrefabComponentID );
#endif

    MyAssert( m_SceneIDLoadedFrom == SCENEID_NotSet || m_SceneIDLoadedFrom == sceneID );
    SetSceneID( sceneID );

    if( m_pGameObject && m_pGameObject->IsEnabled() == false )
        m_EnabledState = EnabledState_Disabled_EnableWithGameObject;

    bool enabled = true;
    cJSONExt_GetBool( jComponent, "Enabled", &enabled );
    SetEnabled( enabled );

    // TODO: this will break if more variables are added to a component or it's parents.
    cJSONExt_GetUnsignedInt( jComponent, "Divorced", &m_DivorcedVariables );

    ImportVariablesFromJSON( jComponent, this, GetComponentVariableList(), this, m_SceneIDLoadedFrom ); //_VARIABLE_LIST
}

void ComponentBase::FinishImportingFromJSONObject(cJSON* jComponent)
{
}

cJSON* ComponentBase::ExportReferenceAsJSONObject() const
{
    // see ComponentSystemManager::FindComponentByJSONRef

    cJSON* ref = m_pGameObject->ExportReferenceAsJSONObject( GetSceneID() );
    MyAssert( ref );

    if( ref )
    {
        cJSON_AddNumberToObject( ref, "ComponentID", m_ID );
    }

    return ref;
}

ComponentBase& ComponentBase::operator=(const ComponentBase& other)
{
    MyAssert( &other != this );

    m_DivorcedVariables = other.m_DivorcedVariables;

#if MYFW_EDITOR
    m_PrefabComponentID = other.m_PrefabComponentID;
#endif

    return *this;
}

void ComponentBase::OnLoad()
{
    if( m_EnabledState == EnabledState_Enabled )
        RegisterCallbacks();
    else
        UnregisterCallbacks();
}

void ComponentBase::OnGameObjectEnabled()
{
    if( m_EnabledState == EnabledState_Enabled )
        return;

    if( m_EnabledState == EnabledState_Disabled_EnableWithGameObject )
    {
        SetEnabled( true );
    }
}

void ComponentBase::OnGameObjectDisabled()
{
    if( m_EnabledState != EnabledState_Enabled )
        return;

    SetEnabled( false );

    // If this component was previously enabled, then the "SetEnabled( false );" call above will set it to ManuallyDisabled.
    // Switch it to "EnabledState_Disabled_EnableWithGameObject".
    if( m_EnabledState == EnabledState_Disabled_ManuallyDisabled )
    {
        m_EnabledState = EnabledState_Disabled_EnableWithGameObject;
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
bool ComponentBase::SetEnabled(bool enableComponent)
{
    bool stateChanged = false;

    if( enableComponent == true )
    {
        // If component as already enabled, kick out.
        if( m_EnabledState == EnabledState_Enabled )
            return false;

        // If the GameObject is disabled, mark component to enable with GameObject, but don't enable it.
        if( m_pGameObject && m_pGameObject->IsEnabled() == false )
        {
            m_EnabledState = EnabledState_Disabled_EnableWithGameObject;
            return false;
        }

        // Register callbacks if the component needs to be enabled.
        m_EnabledState = EnabledState_Enabled;
        RegisterCallbacks();
        stateChanged = true;
    }
    else
    {
        // If the component was already manually disabled, kick out.
        if( m_EnabledState == EnabledState_Disabled_ManuallyDisabled )
            return false;

        // Only unregister callbacks if the component was enabled.
        if( m_EnabledState == EnabledState_Enabled )
        {
            m_EnabledState = EnabledState_Disabled_ManuallyDisabled;
            UnregisterCallbacks();
            stateChanged = true;
        }
        else
        {
            MyAssert( m_EnabledState == EnabledState_Disabled_EnableWithGameObject );

            // If it was set to "EnableWithGameObject", then set to manually disabled.
            m_EnabledState = EnabledState_Disabled_ManuallyDisabled;
        }
    }

    return stateChanged;
}

const char* ComponentBase::GetTypeName()
{
    return m_pComponentSystemManager->GetComponentTypeManager()->GetTypeName( m_Type );
}

SceneInfo* ComponentBase::GetSceneInfo()
{
    return m_pComponentSystemManager->GetSceneInfo( m_SceneIDLoadedFrom );
}

void ComponentBase::RegisterOnDeleteCallback(void* pObj, ComponentDeletedCallbackFunc* pCallback)
{
    MyAssert( pCallback != 0 );

    // Make sure the same callback isn't being registered.
    for( CPPListNode* pNode = m_pOnDeleteCallbacks.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentDeletedCallbackStruct* pCallbackStruct = (ComponentDeletedCallbackStruct*)pNode;
        if( pCallbackStruct->pObj == pObj && pCallbackStruct->pFunc == pCallback )
            return;
    }

    // TODO: Pool callback structures.
    ComponentDeletedCallbackStruct* pCallbackStruct = MyNew ComponentDeletedCallbackStruct;
    pCallbackStruct->pObj = pObj;
    pCallbackStruct->pFunc = pCallback;

    m_pOnDeleteCallbacks.AddTail( pCallbackStruct );
}

void ComponentBase::UnregisterOnDeleteCallback(void* pObj, ComponentDeletedCallbackFunc* pCallback)
{
    for( CPPListNode* pNode = m_pOnDeleteCallbacks.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentDeletedCallbackStruct* pCallbackStruct = (ComponentDeletedCallbackStruct*)pNode;
        if( pCallbackStruct->pObj == pObj && pCallbackStruct->pFunc == pCallback )
        {
            pCallbackStruct->Remove();
            delete pCallbackStruct;
            return;
        }
    }
}

void ComponentBase::NotifyOthersThisWasDeleted()
{
    for( CPPListNode* pNode = m_pOnDeleteCallbacks.GetHead(); pNode; )
    {
        CPPListNode* pNextNode = pNode->GetNext();

        ComponentDeletedCallbackStruct* pCallbackStruct = (ComponentDeletedCallbackStruct*)pNode;

        // Remove the callback struct from the list before calling the function
        //     since the callback function might try to unregister (and delete) the callback struct.
        pCallbackStruct->Remove();

        // Call the onComponentDeleted callback function.
        pCallbackStruct->pFunc( pCallbackStruct->pObj, this );

        // Delete the struct.
        delete pCallbackStruct;

        pNode = pNextNode;
    }
}

// Static method.
void ComponentBase::ClearAllVariables_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList)
{
    while( CPPListNode* pNode = pComponentVariableList->GetHead() )
    {
        ComponentVariable* pVariable = (ComponentVariable*)pNode;
        pVariable->Remove();
        delete pVariable;
    }
}

// Static method.
ComponentVariable* ComponentBase::AddVariable_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc)
{
    ComponentVariable* pVariable = MyNew ComponentVariable( label, type, offset, saveload, displayinwatch, watchlabel,
        0, 0, 0, 0, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc );
    
    if( pComponentVariableList->GetTail() == 0 )
        pVariable->m_Index = 0;
    else
        pVariable->m_Index = ((ComponentVariable*)pComponentVariableList->GetTail())->m_Index + 1;

    pComponentVariableList->AddTail( pVariable );

    return pVariable;
}

// Static method.
ComponentVariable* ComponentBase::AddVariablePointer_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_GetPointerValue pGetPointerValueCallBackFunc, CVarFunc_SetPointerValue pSetPointerValueCallBackFunc, CVarFunc_GetPointerDesc pGetPointerDescCallBackFunc, CVarFunc_SetPointerDesc pSetPointerDescCallBackFunc, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc)
{
    ComponentVariable* pVariable = MyNew ComponentVariable( label, ComponentVariableType_PointerIndirect, -1, saveload, displayinwatch, watchlabel,
        pGetPointerValueCallBackFunc, pSetPointerValueCallBackFunc, pGetPointerDescCallBackFunc, pSetPointerDescCallBackFunc,
        pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc );

    if( pComponentVariableList->GetTail() == 0 )
        pVariable->m_Index = 0;
    else
        pVariable->m_Index = ((ComponentVariable*)pComponentVariableList->GetTail())->m_Index + 1;

    pComponentVariableList->AddTail( pVariable );

    return pVariable;
}

// Static method.
ComponentVariable* ComponentBase::AddVariableEnum_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc)
{
    ComponentVariable* pVariable = AddVariable_Base( pComponentVariableList, label, ComponentVariableType_Enum, offset, saveload, displayinwatch,
        watchlabel, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc );
    
    pVariable->m_NumEnumStrings = numenums;
    pVariable->m_ppEnumStrings = ppStrings;

    return pVariable;
}

// Static method.
ComponentVariable* ComponentBase::AddVariableFlags_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc)
{
    ComponentVariable* pVariable = AddVariable_Base( pComponentVariableList, label, ComponentVariableType_Flags, offset, saveload, displayinwatch,
        watchlabel, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc );
    
    pVariable->m_NumEnumStrings = numenums;
    pVariable->m_ppEnumStrings = ppStrings;

    return pVariable;
}

#if MYFW_USING_WX
void ComponentBase::FillPropertiesWindowWithVariables()
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        AddVariableToPropertiesWindow( pVar );
    }
}
#endif //MYFW_USING_WX

#if MYFW_EDITOR
bool ComponentBase::DoAllMultiSelectedVariabledHaveTheSameValue(ComponentVariable* pVar)
{
    bool allComponentsHaveSameValue = true;

    //if( pVar->m_Offset != -1 )
    {
        switch( pVar->m_Type )
        {
        case ComponentVariableType_Int:
        case ComponentVariableType_Enum:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(int*)((char*)this + pVar->m_Offset) != *(int*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_Flags:
        case ComponentVariableType_UnsignedInt:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(unsigned int*)((char*)this + pVar->m_Offset) != *(unsigned int*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        //ComponentVariableType_Char,
        //ComponentVariableType_UnsignedChar,

        case ComponentVariableType_Bool:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(bool*)((char*)this + pVar->m_Offset) != *(bool*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_Float:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(float*)((char*)this + pVar->m_Offset) != *(float*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        //ComponentVariableType_Double,
        //ComponentVariableType_ColorFloat,

        case ComponentVariableType_ColorByte:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(ColorByte*)((char*)this + pVar->m_Offset) != *(ColorByte*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_Vector2:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(Vector2*)((char*)this + pVar->m_Offset) != *(Vector2*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_Vector3:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(Vector3*)((char*)this + pVar->m_Offset) != *(Vector3*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_Vector2Int:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(Vector2Int*)((char*)this + pVar->m_Offset) != *(Vector2Int*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_Vector3Int:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(Vector3Int*)((char*)this + pVar->m_Offset) != *(Vector3Int*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_GameObjectPtr:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( (GameObject*)((char*)this + pVar->m_Offset) != (GameObject*)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_ComponentPtr:
        case ComponentVariableType_FilePtr:
        case ComponentVariableType_MaterialPtr:
        case ComponentVariableType_TexturePtr:
        case ComponentVariableType_SoundCuePtr:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( *(void**)((char*)this + pVar->m_Offset) != *(void**)((char*)m_MultiSelectedComponents[i] + pVar->m_Offset) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_PointerIndirect:
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( (this->*pVar->m_pGetPointerValueCallBackFunc)( pVar ) != (m_MultiSelectedComponents[i]->*pVar->m_pGetPointerValueCallBackFunc)( pVar ) )
                    allComponentsHaveSameValue = false;
            }
            break;

        case ComponentVariableType_NumTypes:
        default:
            MyAssert( false );
            break;
        }
    }

    return allComponentsHaveSameValue;
}
#endif //MYFW_EDITOR

#if MYFW_USING_IMGUI
void ComponentBase::AddAllVariablesToWatchPanel()
{
    TCPPListHead<ComponentVariable*>* pComponentVariableList = GetComponentVariableList();
    if( pComponentVariableList == 0 )
        return;

    for( CPPListNode* pNode = pComponentVariableList->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        AddVariableToWatchPanel( m_pEngineCore, this, pVar, this, nullptr );
    }
}

// Static method.
void ComponentBase::TestForVariableModificationAndCreateUndoCommand(void* pObject, EngineCore* pEngineCore, ImGuiID id, bool modified, ComponentVariable* pVar, ComponentBase* pObjectAsComponent, CommandStack* pCommandStack)
{
    // These statics are used by non-component variables, currently used by the node graph system.
    // Should be okay as globals unless we have 2 inputs interacting with 2 imgui controls simultaneously.
    static ComponentVariableValue variableValueWhenControlSelected;
    static bool linkNextUndoCommandToPrevious = false;
    static ImGuiID lastImGuiID = 0;

    if( pCommandStack == nullptr )
    {
        pCommandStack = pEngineCore->GetCommandStack();
    }

    // If the id passed in is different than the last known value, then assume a new control was selected.
    if( pObjectAsComponent )
    {
        pEngineCore = pObjectAsComponent->m_pEngineCore;

        if( id != pObjectAsComponent->m_ImGuiControlIDForCurrentlySelectedVariable )
        {
            // If a new control was selected, store the starting value and start a new undo chain.
            pObjectAsComponent->m_ComponentVariableValueWhenControlSelected.GetValueFromVariable( pObject, pVar, pObjectAsComponent );
            pObjectAsComponent->m_LinkNextUndoCommandToPrevious = false;
            pObjectAsComponent->m_ImGuiControlIDForCurrentlySelectedVariable = id;
        }
    }
    else
    {
        if( id != lastImGuiID )
        {
            variableValueWhenControlSelected = ComponentVariableValue( pObject, pVar, pObjectAsComponent );
            linkNextUndoCommandToPrevious = false;
            lastImGuiID = id;
        }
    }

    // If the control returned true to indicate it was modified, then create an undo command.
    if( modified && id != 0 )
    {
        MyAssert( pObjectAsComponent == nullptr || id == pObjectAsComponent->m_ImGuiControlIDForCurrentlySelectedVariable );

        // Store the end value.
        ComponentVariableValue endValue( pObject, pVar, pObjectAsComponent );

        if( pObjectAsComponent )
        {
            // Add an undo action.
            pCommandStack->Do(
                MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                                    pObject, pVar, endValue, pObjectAsComponent->m_ComponentVariableValueWhenControlSelected, true, pObjectAsComponent ),
                pObjectAsComponent->m_LinkNextUndoCommandToPrevious );

            // Link the next undo command to this one.
            // TODO: since we're passing in the starting value,
            //       we can actually replace the old command rather than link to it.
            pObjectAsComponent->m_LinkNextUndoCommandToPrevious = true;
        }
        else
        {
            // Safety check, the previous command should use the same object as the current one if we want to link them.
            if( linkNextUndoCommandToPrevious == true )
            {
                bool allowedToLinkToPrevious = false;

                // If the previous command is changing the same object and variable, then link to it.
                if( pCommandStack->GetUndoStackSize() > 0 )
                {
                    EditorCommand* pPreviousCommand = pCommandStack->GetUndoCommandAtIndex( pCommandStack->GetUndoStackSize() - 1 );
                    if( strcmp( pPreviousCommand->GetName(), "EditorCommand_ImGuiPanelWatchNumberValueChanged" ) == 0 )
                    {
                        EditorCommand_ImGuiPanelWatchNumberValueChanged* pCommand = (EditorCommand_ImGuiPanelWatchNumberValueChanged*)pPreviousCommand;
                        if( pCommand->UsesThisObjectAndVariable( pObject, pVar ) )
                        {
                            allowedToLinkToPrevious = true;
                        }
                    }
                }

                if( allowedToLinkToPrevious == false )
                    linkNextUndoCommandToPrevious = false;
            }

            // Add an undo action.
            pCommandStack->Do(
                MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                                    pObject, pVar, endValue, variableValueWhenControlSelected, true, nullptr ),
                linkNextUndoCommandToPrevious );

            linkNextUndoCommandToPrevious = true;
        }
    }
}

// Static method.
bool ComponentBase::AddVariableToWatchPanel(EngineCore* pEngineCore, void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent, CommandStack* pCommandStack)
{
    if( pCommandStack == nullptr )
    {
        pCommandStack = pEngineCore->GetCommandStack();
    }

    bool modified = false;

    MyAssert( pObject != nullptr );

    if( pVar->m_DisplayInWatch == false )
    {
    //    // clear out the control id, for cases where variables get dynamically enabled/disabled.
    //    pVar->m_ControlID = -10; // less than -4 since vec4's add 3 in FindComponentVariableForControl()
        return modified;
    }

    if( pVar->m_pShouldVariableBeAddedCallbackFunc )
    {
        if( pObjectAsComponent && (pObjectAsComponent->*pVar->m_pShouldVariableBeAddedCallbackFunc)( pVar ) == false )
        {
            pVar->m_ControlID = -10; // less than -4 since vec4's add 3 in FindComponentVariableForControl()
            return modified;
        }
    }

    int numStylesPushed = 0;

    if( pObjectAsComponent && pObjectAsComponent->IsDivorced( pVar->m_Index ) )
    {
        Vector4 color = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_DivorcedVarText );
        ImGui::PushStyleColor( ImGuiCol_Text, color ); //ImVec4( 1.0f, 0.5f, 0.0f, 1.0f ) );
        numStylesPushed++;
    }

    if( pObjectAsComponent && pObjectAsComponent->DoAllMultiSelectedVariabledHaveTheSameValue( pVar ) == false )
    {
        Vector4 color = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_MultiSelectedVarDiffText );
        ImGui::PushStyleColor( ImGuiCol_Text, color ); //ImVec4( 0.0f, 0.5f, 1.0f, 1.0f ) );
        numStylesPushed++;
    }

    if( pVar->m_Label != 0 )
        ImGui::PushID( pVar->m_Label );

    {
        unsigned int contextMenuItemCount = 0;

        switch( pVar->m_Type )
        {
        case ComponentVariableType_Int:
            {
                float speed = 1.0f;
                modified = ImGui::DragInt( pVar->m_WatchLabel, (int*)((char*)pObject + pVar->m_Offset), speed, (int)pVar->m_FloatLowerLimit, (int)pVar->m_FloatUpperLimit );
                ComponentBase::TestForVariableModificationAndCreateUndoCommand( pObject, pEngineCore, ImGuiExt::GetActiveItemId(), modified, pVar, pObjectAsComponent, pCommandStack );
            }
            break;

        case ComponentVariableType_Enum:
            {
                const char** items = pVar->m_ppEnumStrings;
                int currentItem = *(int*)((char*)pObject + pVar->m_Offset);
                const char* currentItemStr = pVar->m_ppEnumStrings[currentItem];
                if( ImGui::BeginCombo( pVar->m_WatchLabel, currentItemStr ) )
                {
                    for( int n = 0; n < pVar->m_NumEnumStrings; n++ )
                    {
                        bool is_selected = (n == currentItem);
                        if( ImGui::Selectable( items[n], is_selected ) )
                        {
                            // Store the old value.
                            ComponentVariableValue oldvalue( pObject, pVar, pObjectAsComponent );

                            // Change the value.
                            *(int*)((char*)pObject + pVar->m_Offset) = n;

                            // Store the new value.
                            ComponentVariableValue newvalue( pObject, pVar, pObjectAsComponent );

                            pCommandStack->Do(
                                MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged( pObject, pVar, newvalue, oldvalue, true, pObjectAsComponent ),
                                false );

                            modified = true;
                        }
                        if( is_selected )
                        {
                            // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            break;

        case ComponentVariableType_Flags:
            {
                const char** items = pVar->m_ppEnumStrings;
                unsigned int flags = *(int*)((char*)pObject + pVar->m_Offset);

                // Build a string of all selected flags.
                std::string str;
                int count = 0;
                for( int i=0; i<pVar->m_NumEnumStrings; i++ )
                {
                    if( flags & 1<<i )
                    {
                        if( count != 0 )
                            str.append( "," );

                        str.append( pVar->m_ppEnumStrings[i] );

                        count++;
                    }
                }

                // Display the combo box with checkboxes for each label.
                if( ImGui::BeginCombo( pVar->m_WatchLabel, str.c_str() ) )
                {
                    for( int n = 0; n < pVar->m_NumEnumStrings; n++ )
                    {
                        bool is_selected = (flags & 1<<n) > 0;
                        if( ImGui::CheckboxFlags( items[n], &flags, 1<<n ) )
                        {
                            // Store the old value.
                            ComponentVariableValue oldvalue( pObject, pVar, pObjectAsComponent );

                            // Change the value.
                            if( is_selected )
                                *(int*)((char*)pObject + pVar->m_Offset) &= ~(1<<n);
                            else
                                *(int*)((char*)pObject + pVar->m_Offset) |= 1<<n;

                            // Store the new value.
                            ComponentVariableValue newvalue( pObject, pVar, pObjectAsComponent );

                            if( pObjectAsComponent )
                            {
                                pCommandStack->Do(
                                    MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged( pObject, pVar, newvalue, oldvalue, true, pObjectAsComponent ),
                                    false );
                            }
                        }
                        if( is_selected )
                        {
                            // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                            // Initial focus will be on the last selected item.
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            break;

        case ComponentVariableType_UnsignedInt:
            {
                unsigned int* pValueUInt = (unsigned int*)((char*)pObject + pVar->m_Offset);
                int valueInt = *pValueUInt;

                float speed = 1.0f;
                if( ImGui::DragInt( pVar->m_WatchLabel, &valueInt, speed, (int)pVar->m_FloatLowerLimit, (int)pVar->m_FloatUpperLimit ) )
                {
                    *pValueUInt = valueInt;
                }
            }
            break;

        //ComponentVariableType_Char,
        //ComponentVariableType_UnsignedChar,
        case ComponentVariableType_Bool:
            {
                modified = ImGui::Checkbox( pVar->m_WatchLabel, (bool*)((char*)pObject + pVar->m_Offset) );
                if( modified )
                {
                    // Flip the bool to store the old value, then flip is back.
                    *(bool*)((char*)pObject + pVar->m_Offset) = !*(bool*)((char*)pObject + pVar->m_Offset);
                    ComponentVariableValue oldvalue( pObject, pVar, pObjectAsComponent );
                    *(bool*)((char*)pObject + pVar->m_Offset) = !*(bool*)((char*)pObject + pVar->m_Offset);

                    // Store the new value.
                    ComponentVariableValue newvalue( pObject, pVar, pObjectAsComponent );

                    if( pObjectAsComponent )
                    {
                        pCommandStack->Do(
                            MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged( pObject, pVar, newvalue, oldvalue, true, pObjectAsComponent ),
                            false );
                    }
                }
            }
            break;

        case ComponentVariableType_Float:
            {
                float speed = 0.1f;
                if( pVar->m_FloatUpperLimit - pVar->m_FloatLowerLimit > 0 )
                    speed = (pVar->m_FloatUpperLimit - pVar->m_FloatLowerLimit) / 300.0f;
                modified = ImGui::DragFloat( pVar->m_WatchLabel, (float*)((char*)pObject + pVar->m_Offset), speed, pVar->m_FloatLowerLimit, pVar->m_FloatUpperLimit );
                ComponentBase::TestForVariableModificationAndCreateUndoCommand( pObject, pEngineCore, ImGuiExt::GetActiveItemId(), modified, pVar, pObjectAsComponent, pCommandStack );
            }
            break;

        //ComponentVariableType_Double,
        //ComponentVariableType_ColorFloat,

        case ComponentVariableType_ColorByte:
            {
                ColorFloat colorFloat = ((ColorByte*)((char*)pObject + pVar->m_Offset))->AsColorFloat();
                modified = ImGui::ColorEdit4( pVar->m_WatchLabel, &colorFloat.r );
                if( modified )
                {
                    *(ColorByte*)((char*)pObject + pVar->m_Offset) = colorFloat.AsColorByte();
                }

                ComponentBase::TestForVariableModificationAndCreateUndoCommand( pObject, pEngineCore, ImGuiExt::GetActiveItemId(), modified, pVar, pObjectAsComponent, pCommandStack );
            }
            break;

        case ComponentVariableType_Vector2:
            {
                modified = ImGui::DragFloat2( pVar->m_WatchLabel, (float*)((char*)pObject + pVar->m_Offset), 0.1f, pVar->m_FloatLowerLimit, pVar->m_FloatUpperLimit );
                if( pObjectAsComponent )
                {
                    ComponentBase::TestForVariableModificationAndCreateUndoCommand( pObject, pEngineCore, ImGuiExt::GetActiveItemId(), modified, pVar, pObjectAsComponent, pCommandStack );
                }
            }
            break;

        case ComponentVariableType_Vector3:
            {
                modified = ImGui::DragFloat3( pVar->m_WatchLabel, (float*)((char*)pObject + pVar->m_Offset), 0.1f, pVar->m_FloatLowerLimit, pVar->m_FloatUpperLimit );
                if( pObjectAsComponent )
                {
                    ComponentBase::TestForVariableModificationAndCreateUndoCommand( pObject, pEngineCore, ImGuiExt::GetActiveItemId(), modified, pVar, pObjectAsComponent, pCommandStack );
                }
            }
            break;

        case ComponentVariableType_Vector2Int:
            {
                modified = ImGui::DragInt2( pVar->m_WatchLabel, (int*)((char*)pObject + pVar->m_Offset) );
                if( pObjectAsComponent )
                {
                    ComponentBase::TestForVariableModificationAndCreateUndoCommand( pObject, pEngineCore, ImGuiExt::GetActiveItemId(), modified, pVar, pObjectAsComponent, pCommandStack );
                }
            }
            break;

        case ComponentVariableType_Vector3Int:
            ImGui::DragInt3( pVar->m_WatchLabel, (int*)((char*)pObject + pVar->m_Offset) );
            //pVar->m_ControlID = g_pPanelWatch->AddVector3Int( pVar->m_WatchLabel, (Vector3Int*)((char*)this + pVar->m_Offset), 0.0f, 0.0f, this, ComponentBase::StaticOnValueChangedVariable, ComponentBase::StaticOnRightClickVariable );
            break;

        case ComponentVariableType_GameObjectPtr:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                GameObject* pGameObject = *(GameObject**)((char*)pObject + pVar->m_Offset);

                const char* pDesc = "none";
                if( pGameObject )
                {
                    pDesc = pGameObject->GetName();
                }

                float width = ImGui::GetWindowWidth() * 0.65f;
                if( pObjectAsComponent == nullptr )
                {
                    width = 117.0f;
                }

                if( ImGui::Button( pDesc, ImVec2( width, 0 ) ) )
                {
                    // TODO: Pop up a component picker window.
                }

                if( ImGui::BeginDragDropTarget() )
                {
                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "GameObject" ) )
                    {
                        GameObject* pGameObject = (GameObject*)*(void**)payload->Data;

                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pGameObject );

                        if( pObjectAsComponent )
                        {
                            pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                        }
                        else
                        {
                            ComponentVariableCallbackInterface* pCallbackObject = (ComponentVariableCallbackInterface*)pObject;
                            (pCallbackObject->*pVar->m_pOnDropCallbackFunc)( pVar, true, -1, -1 );
                        }
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::Text( pVar->m_WatchLabel );

                ImGui::EndGroup();
            }
            break;

        case ComponentVariableType_ComponentPtr:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                ComponentBase* pComponent = *(ComponentBase**)((char*)pObject + pVar->m_Offset);

                const char* pDesc = "none";
                if( pComponent )
                {
                    pDesc = pComponent->m_pGameObject->GetName();
                }

                float width = ImGui::GetWindowWidth() * 0.65f;
                if( pObjectAsComponent == nullptr )
                {
                    width = 117.0f;
                }

                if( ImGui::Button( pDesc, ImVec2( width, 0 ) ) )
                {
                    // TODO: Pop up a component picker window.
                }

                if( ImGui::BeginDragDropTarget() )
                {
                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "GameObject" ) )
                    {
                        GameObject* pGameObject = (GameObject*)*(void**)payload->Data;

                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pGameObject );

                        if( pObjectAsComponent )
                        {
                            pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                        }
                        else
                        {
                            ComponentVariableCallbackInterface* pCallbackObject = (ComponentVariableCallbackInterface*)pObject;
                            (pCallbackObject->*pVar->m_pOnDropCallbackFunc)( pVar, true, -1, -1 );
                        }
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::Text( pVar->m_WatchLabel );

                ImGui::EndGroup();
            }
            break;

        case ComponentVariableType_FilePtr:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                MyFileObject* pFile = *(MyFileObject**)((char*)pObject + pVar->m_Offset);

                Vector4 buttonColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectButton );
                Vector4 textColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectText );

                const char* pDesc = "none";
                if( pFile )
                {
                    pDesc = pFile->GetFullPath();
                    buttonColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Button );
                    textColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                }

                ImGui::PushStyleColor( ImGuiCol_Button, buttonColor );
                ImGui::PushStyleColor( ImGuiCol_Text, textColor );
                if( ImGui::Button( pDesc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) ) )
                {
                    // TODO: pop up a lua script file picker window.
                }
                ImGui::PopStyleColor( 2 );

                if( ImGui::BeginDragDropTarget() )
                {
                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "File" ) )
                    {
                        MyFileObject* pNewFile = (MyFileObject*)*(void**)payload->Data;

                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_FileObjectPointer, pNewFile );

                        if( pObjectAsComponent )
                        {
                            pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                        }
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::Text( pVar->m_WatchLabel );

                ImGui::EndGroup();
            }
            break;

        case ComponentVariableType_MaterialPtr:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                MaterialDefinition* pMaterial = *(MaterialDefinition**)((char*)pObject + pVar->m_Offset);

                const char* pDesc = "no material";
                if( pMaterial != 0 )
                    pDesc = pMaterial->GetName();

                if( ImGui::Button( pDesc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) ) )
                {
                    // TODO: pop up a material picker window.
                }

                if( ImGui::BeginDragDropTarget() )
                {
                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Material" ) )
                    {
                        MaterialDefinition* pNewMat = (MaterialDefinition*)*(void**)payload->Data;

                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_MaterialDefinitionPointer, pNewMat );

                        if( pObjectAsComponent )
                        {
                            pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                        }
                    }

                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "File" ) )
                    {
                        //(this->*pVar->m_pSetPointerValueCallBackFunc)( pVar, *(void**)payload->Data );
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::Text( pVar->m_WatchLabel );

                ImGui::EndGroup();
            }
            break;

        case ComponentVariableType_TexturePtr:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                TextureDefinition* pTexture = *(TextureDefinition**)((char*)pObject + pVar->m_Offset);

                const char* pDesc = "no texture";
                if( pTexture != 0 )
                    pDesc = pTexture->GetFilename();

                if( ImGui::Button( pDesc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) ) )
                {
                    // TODO: pop up a texture picker window.
                }

                if( ImGui::BeginDragDropTarget() )
                {
                    TextureDefinition* pTextureDropped = nullptr;

                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Texture" ) )
                    {
                        pTextureDropped = (TextureDefinition*)*(void**)payload->Data;
                    }

                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "File" ) )
                    {
                        if( pObjectAsComponent )
                        {
                            TextureManager* pTextureManager = pObjectAsComponent->m_pEngineCore->GetManagers()->GetTextureManager();

                            MyFileObject* pFile = (MyFileObject*)*(void**)payload->Data;
                            MyAssert( pFile );
                            pTextureDropped = pTextureManager->FindTexture( pFile );
                        }
                    }

                    if( pTextureDropped != nullptr )
                    {
                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_TextureDefinitionPointer, pTextureDropped );

                        if( pObjectAsComponent )
                        {
                            pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                        }
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::Text( pVar->m_WatchLabel );

                ImGui::EndGroup();
            }
            break;

        case ComponentVariableType_SoundCuePtr:
            {
                SoundCue* pCue = *(SoundCue**)((char*)pObject + pVar->m_Offset);

                const char* desc = "no sound cue";
                if( pCue != 0 )
                    desc = pCue->GetName();

                ImGui::Text( "FilePtr: %s: %s (TODO)", pVar->m_WatchLabel, desc );
                //pVar->m_ControlID = g_pPanelWatch->AddPointerWithDescription( pVar->m_WatchLabel, pCue, desc, this, ComponentBase::StaticOnDropVariable, ComponentBase::StaticOnValueChangedVariable, ComponentBase::StaticOnRightClickVariable );
            }
            break;

        case ComponentVariableType_PointerIndirect:
            if( pObjectAsComponent )
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                //void* pPtr = (this->*pVar->m_pGetPointerValueCallBackFunc)( pVar );
                const char* pDesc = (pObjectAsComponent->*pVar->m_pGetPointerDescCallBackFunc)( pVar );

                Vector4 buttonColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectButton );
                Vector4 textColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectText );

                if( pDesc != 0 && pDesc[0] != 0 )
                {
                    buttonColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Button );
                    textColor = pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                }

                ImGui::PushStyleColor( ImGuiCol_Button, buttonColor );
                ImGui::PushStyleColor( ImGuiCol_Text, textColor );
                if( ImGui::Button( pDesc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) ) )
                {
                    // TODO: pop up a material picker window.
                }
                ImGui::PopStyleColor( 2 );

                if( ImGui::BeginDragDropTarget() )
                {
                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Material" ) )
                    {
                        MaterialDefinition* pMat = (MaterialDefinition*)*(void**)payload->Data;

                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_MaterialDefinitionPointer, pMat );

                        pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                    }

                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "File" ) )
                    {
                        MyFileObject* pNewFile = (MyFileObject*)*(void**)payload->Data;

                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_FileObjectPointer, pNewFile );

                        pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                    }

                    ImGui::EndDragDropTarget();
                }

                // Clear material on double click.
                if( ImGui::IsItemHovered() )
                {
                    if( ImGui::IsMouseDoubleClicked( 0 ) )
                    {
                        // Clear material by "dropping" at null material on the object.
                        // This will create undo along with update divorce status and children.
                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                        g_DragAndDropStruct.Add( DragAndDropType_MaterialDefinitionPointer, 0 );

                        pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                    }
                }

                ImGui::SameLine();
                ImGui::Text( pVar->m_WatchLabel );

                ImGui::EndGroup();
            }
            break;

        case ComponentVariableType_NumTypes:
        //default:
            MyAssert( false );
            break;
        }

        ImGui::PopStyleColor( numStylesPushed );
        numStylesPushed = 0;

        // Right-click menu, for divorce/marry and other things.
        // Will attach to last control used, which should be the control and label which are grouped.
        if( pObjectAsComponent )
        {
            ImGui::PushID( pVar );
            if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
            {
                // Add divorce/marry options first.
                if( pObjectAsComponent->m_pGameObject->GetGameObjectThisInheritsFrom() )
                {
                    if( pObjectAsComponent->IsDivorced( pVar->m_Index ) == false )
                    {
                        contextMenuItemCount++;
                        if( ImGui::MenuItem( "Divorce value from parent" ) )
                        {
                            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DivorceOrMarryComponentVariable( pObjectAsComponent, pVar, true ) );

                            ImGui::CloseCurrentPopup();
                        }
                    }
                    else
                    {                        
                        contextMenuItemCount++;
                        if( ImGui::MenuItem( "Reset value to parent" ) )
                        {
                            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DivorceOrMarryComponentVariable( pObjectAsComponent, pVar, false ) );

                            ImGui::CloseCurrentPopup();
                        }
                    }
                }

                // Add material options.
                if( pVar->m_Type == ComponentVariableType_MaterialPtr )
                {
                    MaterialDefinition* pMaterial = *(MaterialDefinition**)((char*)pObject + pVar->m_Offset);

                    if( pMaterial )
                    {
                        if( ImGui::MenuItem( "Clear material" ) )
                        {
                            // Clear the current material by "dropping" a null material.  Will deal with inheritance/divorce/etc.
                            g_DragAndDropStruct.Clear();
                            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                            g_DragAndDropStruct.Add( DragAndDropType_MaterialDefinitionPointer, 0 );

                            if( pObjectAsComponent )
                            {
                                pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                            }
                        }

                        contextMenuItemCount++;
                    }

                    // TODO: Have the callback return how many menu items it added instead of assuming it added one.
                    pEngineCore->GetEditorMainFrame_ImGui()->AddContextMenuItemsForMaterials( pMaterial );

                    contextMenuItemCount++;
                }

                // Add texture options.
                if( pVar->m_Type == ComponentVariableType_TexturePtr )
                {
                    TextureDefinition* pTexture = *(TextureDefinition**)((char*)pObject + pVar->m_Offset);

                    if( pTexture )
                    {
                        if( ImGui::MenuItem( "Clear texture" ) )
                        {
                            // Clear the current texture by "dropping" a null texture.  Will deal with inheritance/divorce/etc.
                            g_DragAndDropStruct.Clear();
                            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                            g_DragAndDropStruct.Add( DragAndDropType_TextureDefinitionPointer, 0 );

                            if( pObjectAsComponent )
                            {
                                pObjectAsComponent->OnDropVariable( pVar, 0, -1, -1, true );
                            }
                        }

                        contextMenuItemCount++;
                    }

                    // TODO: Have the callback return how many menu items it added instead of assuming it added one.
                    //pEngineCore->GetEditorMainFrame_ImGui()->AddContextMenuItemsForTextures( pTexture );

                    contextMenuItemCount++;
                }

                // Call this pVar callback, not sure what's using it.
                if( pVar->m_pOnRightClickCallbackFunc )
                {
                    (pObjectAsComponent->*(pVar->m_pOnRightClickCallbackFunc))( pVar );

                    // TODO: Have the callback return how many menu items it added instead of assuming it added one.
                    contextMenuItemCount++;
                }

                // Add generic "file" options last.
                if( pVar->m_Type == ComponentVariableType_FilePtr )
                {
                    if( contextMenuItemCount > 0 )
                        ImGui::Separator();

                    MyFileObject* pFile = *(MyFileObject**)((char*)pObject + pVar->m_Offset);
                    if( pFile )
                    {
                        // TODO: Have the callback return how many menu items it added instead of assuming it added one.
                        pEngineCore->GetEditorMainFrame_ImGui()->AddContextMenuItemsForFiles( pFile );
                    }

                    contextMenuItemCount++;
                }

                // Close the popup if no menu items were added to it.
                if( contextMenuItemCount == 0 )
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
    }

    if( pVar->m_Label != nullptr )
        ImGui::PopID(); // For ImGui::PushID( pVar->m_Label );

    if( pObjectAsComponent && pVar->m_pVariableAddedToInterfaceCallbackFunc )
    {
        (pObjectAsComponent->*pVar->m_pVariableAddedToInterfaceCallbackFunc)( pVar );
    }

    return modified;
}
#endif

void ComponentBase::ExportVariablesToJSON(cJSON* jComponent, void* pObject, TCPPListHead<ComponentVariable*>* pList, ComponentBase* pObjectAsComponent)
{
    // TODO: remove this once GetComponentVariableList() is pure virtual
    if( pList == 0 )
        return;

    for( CPPListNode* pNode = pList->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_SaveLoad == false )
            continue;

        //if( pVar->m_Offset != -1 )
        {
            switch( pVar->m_Type )
            {
            case ComponentVariableType_Int:
                cJSON_AddNumberToObject( jComponent, pVar->m_Label, *(int*)((char*)pObject + pVar->m_Offset) );
                break;

            case ComponentVariableType_Enum:
                {
                    int enumvalue = *(int*)((char*)pObject + pVar->m_Offset);

                    // Save the enum string value.
                    if( enumvalue >= 0 && enumvalue < pVar->m_NumEnumStrings )
                    {
                        cJSON_AddStringToObject( jComponent, pVar->m_Label, pVar->m_ppEnumStrings[enumvalue] );
                    }
                    else
                    {
                        // if out of range, save as an int.
                        cJSON_AddNumberToObject( jComponent, pVar->m_Label, enumvalue );
                    }
                }
                break;

            case ComponentVariableType_Flags:
                {
                    unsigned int flags = *(unsigned int*)((char*)pObject + pVar->m_Offset);

                    // Save the flags set as strings.
                    if( flags != 0 )
                    {
                        cJSON* jFlagsArray = cJSON_CreateArray();
                        cJSON_AddItemToObject( jComponent, pVar->m_Label, jFlagsArray );
                        for( unsigned int i=0; i<32; i++ )
                        {
                            if( flags & (1<<i) )
                            {
                                MyAssert( pVar->m_ppEnumStrings[i] != 0 );
                                if( pVar->m_ppEnumStrings[i] != 0 )
                                {
                                    cJSON* jFlag = cJSON_CreateString( pVar->m_ppEnumStrings[i] );
                                    cJSON_AddItemToArray( jFlagsArray, jFlag );
                                }
                            }
                        }
                    }
                }
                break;

            case ComponentVariableType_UnsignedInt:
                cJSON_AddNumberToObject( jComponent, pVar->m_Label, *(unsigned int*)((char*)pObject + pVar->m_Offset) );
                break;

            //ComponentVariableType_Char,
            //ComponentVariableType_UnsignedChar,

            case ComponentVariableType_Bool:
                cJSON_AddNumberToObject( jComponent, pVar->m_Label, *(bool*)((char*)pObject + pVar->m_Offset) );
                break;

            case ComponentVariableType_Float:
                cJSON_AddNumberToObject( jComponent, pVar->m_Label, *(float*)((char*)pObject + pVar->m_Offset) );
                break;

            //ComponentVariableType_Double,
            //ComponentVariableType_ColorFloat,

            case ComponentVariableType_ColorByte:
                cJSONExt_AddUnsignedCharArrayToObject( jComponent, pVar->m_Label, (unsigned char*)((char*)pObject + pVar->m_Offset), 4 );
                break;

            case ComponentVariableType_Vector2:
                cJSONExt_AddFloatArrayToObject( jComponent, pVar->m_Label, (float*)((char*)pObject + pVar->m_Offset), 2 );
                break;

            case ComponentVariableType_Vector3:
                cJSONExt_AddFloatArrayToObject( jComponent, pVar->m_Label, (float*)((char*)pObject + pVar->m_Offset), 3 );
                break;

            case ComponentVariableType_Vector2Int:
                cJSONExt_AddIntArrayToObject( jComponent, pVar->m_Label, (int*)((char*)pObject + pVar->m_Offset), 2 );
                break;

            case ComponentVariableType_Vector3Int:
                cJSONExt_AddIntArrayToObject( jComponent, pVar->m_Label, (int*)((char*)pObject + pVar->m_Offset), 3 );
                break;

            case ComponentVariableType_GameObjectPtr:
                {
                    GameObject* pGameObject = *(GameObject**)((char*)pObject + pVar->m_Offset);
                    if( pGameObject )
                    {
                        cJSON_AddNumberToObject( jComponent, pVar->m_Label, pGameObject->GetID() );
                    }
                }
                break;

            case ComponentVariableType_ComponentPtr:
                {
                    ComponentBase* pComponent = *(ComponentBase**)((char*)pObject + pVar->m_Offset);
                    if( pComponent )
                    {
                        cJSON* jComponentRef = pComponent->ExportReferenceAsJSONObject();
                        if( jComponentRef )
                        {
                            cJSON_AddItemToObject( jComponent, pVar->m_Label, jComponentRef );
                        }
                    }
                }
                break;

            case ComponentVariableType_FilePtr:
                {
                    MyFileObject* pFile = *(MyFileObject**)((char*)pObject + pVar->m_Offset);

                    if( pFile != nullptr )
                    {
                        cJSON_AddStringToObject( jComponent, pVar->m_Label, pFile->GetFullPath() );
                    }
                }
                break;

            case ComponentVariableType_MaterialPtr:
            case ComponentVariableType_SoundCuePtr:
                MyAssert( false );
                break;

            case ComponentVariableType_TexturePtr:
                {
                    TextureDefinition* pTexture = *(TextureDefinition**)((char*)pObject + pVar->m_Offset);

                    if( pTexture != nullptr )
                    {
                        cJSON_AddStringToObject( jComponent, pVar->m_Label, pTexture->GetFilename() );
                    }
                }
                break;

            case ComponentVariableType_PointerIndirect:
                MyAssert( pVar->m_pGetPointerDescCallBackFunc );
                if( pVar->m_pGetPointerValueCallBackFunc && pVar->m_pGetPointerDescCallBackFunc )
                {
                    if( (pObjectAsComponent->*pVar->m_pGetPointerValueCallBackFunc)( pVar ) )
                        cJSON_AddStringToObject( jComponent, pVar->m_Label, (pObjectAsComponent->*pVar->m_pGetPointerDescCallBackFunc)( pVar ) );
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

void ComponentBase::ImportVariablesFromJSON(cJSON* jComponent, void* pObject, TCPPListHead<ComponentVariable*>* pList, ComponentBase* pObjectAsComponent, SceneID sceneIDLoadedFrom, const char* singleLabelToImport)
{
    // TODO: Remove this once GetComponentVariableList() is pure virtual.
    if( pList == 0 )
        return;

    ComponentSystemManager* pComponentSystemManager = g_pComponentSystemManager;

    for( CPPListNode* pNode = pList->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_SaveLoad == false )
            continue;

        // If we are looking for a single label to import, check if this is it.
        if( singleLabelToImport != 0 && strcmp( singleLabelToImport, pVar->m_Label ) != 0 )
            continue;

        //if( pVar->m_Offset != -1 )
        {
            switch( pVar->m_Type )
            {
            case ComponentVariableType_Int:
                cJSONExt_GetInt( jComponent, pVar->m_Label, (int*)((char*)pObject + pVar->m_Offset) );
                break;

            case ComponentVariableType_Enum:
                {
                    cJSON* obj = cJSON_GetObjectItem( jComponent, pVar->m_Label );

                    if( obj )
                    {
                        // Try to load the value as a string.
                        if( obj->valuestring != 0 )
                        {
                            for( int i=0; i<pVar->m_NumEnumStrings; i++ )
                            {
                                if( strcmp( pVar->m_ppEnumStrings[i], obj->valuestring ) == 0 )
                                {
                                    *(int*)((char*)pObject + pVar->m_Offset) = i;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // If a string wasn't found, then treat the value as an int.
                            *(int*)((char*)pObject + pVar->m_Offset) = obj->valueint;
                        }
                    }
                }
                break;

            case ComponentVariableType_Flags:
                {
                    cJSON* jFlagsArray = cJSON_GetObjectItem( jComponent, pVar->m_Label );

                    // Load each array value as a string.
                    if( jFlagsArray )
                    {
                        int arraysize = cJSON_GetArraySize( jFlagsArray );
                        if( arraysize == 0 )
                        {
                            // TODO: Remove eventually.
                            // Support for old scene files that didn't store visibility flags in arrays.
                            // Just load valueint as a single flag.
                            *(unsigned int*)((char*)pObject + pVar->m_Offset) = 1<<(jFlagsArray->valueint-1);
                        }
                        else
                        {
                            unsigned int flags = 0;

                            for( int i=0; i<arraysize; i++ )
                            {
                                cJSON* jFlag = cJSON_GetArrayItem( jFlagsArray, i );

                                if( jFlag->valuestring != 0 )
                                {
                                    bool found = false;
                                    for( int i=0; i<pVar->m_NumEnumStrings; i++ )
                                    {
                                        if( strcmp( pVar->m_ppEnumStrings[i], jFlag->valuestring ) == 0 )
                                        {
                                            flags |= 1<<i;
                                            found = true;
                                            break;
                                        }
                                    }

                                    // If the flag string wasn't found, popup a warning.
#if MYFW_USING_WX
                                    if( found == false )
                                    {
                                        static bool messageboxshown = false;
                                        if( messageboxshown == false )
                                        {
                                            messageboxshown = true;
                                            wxMessageBox( "Warning: At least one GameObject Flag string wasn't found,\nsaving the scene will discard old flags", "Warning" );
                                        }
                                    }
#endif //MYFW_USING_WX
                                }
                            }

                            *(unsigned int*)((char*)pObject + pVar->m_Offset) = flags;
                        }
                    }
                }
                break;

            case ComponentVariableType_UnsignedInt:
                cJSONExt_GetUnsignedInt( jComponent, pVar->m_Label, (unsigned int*)((char*)pObject + pVar->m_Offset) );
                break;

            //ComponentVariableType_Char,
            //ComponentVariableType_UnsignedChar,

            case ComponentVariableType_Bool:
                cJSONExt_GetBool( jComponent, pVar->m_Label, (bool*)((char*)pObject + pVar->m_Offset) );
                break;

            case ComponentVariableType_Float:
                cJSONExt_GetFloat( jComponent, pVar->m_Label, (float*)((char*)pObject + pVar->m_Offset) );
                break;

            //ComponentVariableType_Double,
            //ComponentVariableType_ColorFloat,

            case ComponentVariableType_ColorByte:
                cJSONExt_GetUnsignedCharArray( jComponent, pVar->m_Label, (unsigned char*)((char*)pObject + pVar->m_Offset), 4 );
                break;

            case ComponentVariableType_Vector2:
                cJSONExt_GetFloatArray( jComponent, pVar->m_Label, (float*)((char*)pObject + pVar->m_Offset), 2 );
                break;

            case ComponentVariableType_Vector3:
                cJSONExt_GetFloatArray( jComponent, pVar->m_Label, (float*)((char*)pObject + pVar->m_Offset), 3 );
                break;

            case ComponentVariableType_Vector2Int:
                cJSONExt_GetIntArray( jComponent, pVar->m_Label, (int*)((char*)pObject + pVar->m_Offset), 2 );
                break;

            case ComponentVariableType_Vector3Int:
                cJSONExt_GetIntArray( jComponent, pVar->m_Label, (int*)((char*)pObject + pVar->m_Offset), 3 );
                break;

            case ComponentVariableType_GameObjectPtr:
                {
                    unsigned int parentid = 0;
                    cJSONExt_GetUnsignedInt( jComponent, pVar->m_Label, &parentid );
                    if( parentid != 0 )
                    {
                        GameObject* pParentGameObject = pComponentSystemManager->FindGameObjectByID( sceneIDLoadedFrom, parentid );
                        *(GameObject**)((char*)pObject + pVar->m_Offset) = pParentGameObject;
                    }
                }
                break;

            case ComponentVariableType_ComponentPtr:
                {
                    cJSON* jComponentRef = cJSON_GetObjectItem( jComponent, pVar->m_Label );
                    if( jComponentRef )
                    {
                        ComponentBase* pComponent = pComponentSystemManager->FindComponentByJSONRef( jComponentRef, sceneIDLoadedFrom );
                        *(ComponentBase**)((char*)pObject + pVar->m_Offset) = pComponent;
                    }
                }
                break;

            case ComponentVariableType_FilePtr:
                {
                    cJSON* jFileName = cJSON_GetObjectItem( jComponent, pVar->m_Label );

                    if( jFileName )
                    {
                        // Find the file, load if needed.
                        FileManager* pFileManager = pComponentSystemManager->GetEngineCore()->GetManagers()->GetFileManager();
                        MyFileObject* pFile = pFileManager->FindFileByName( jFileName->valuestring );
                        if( pFile == nullptr )
                        {
                            MyFileInfo* pFileInfo = pComponentSystemManager->LoadDataFile( jFileName->valuestring, sceneIDLoadedFrom, nullptr, false );
                            pFile = pFileInfo->GetFile();
                        }

                        // Assign the file to the component.
                        if( pFile != nullptr )
                        {
                            MyFileObject** ppFile = (MyFileObject**)((char*)pObject + pVar->m_Offset);
                            pFile->AddRef();
                            SAFE_RELEASE( *ppFile );
                            *ppFile = pFile;
                        }
                    }
                }
                break;

            case ComponentVariableType_MaterialPtr:
            case ComponentVariableType_SoundCuePtr:
                MyAssert( false );
                break;

            case ComponentVariableType_TexturePtr:
                {
                    cJSON* jTextureName = cJSON_GetObjectItem( jComponent, pVar->m_Label );

                    if( jTextureName )
                    {
                        // Find the texture, load if needed.
                        TextureManager* pTextureManager = pComponentSystemManager->GetEngineCore()->GetManagers()->GetTextureManager();
                        TextureDefinition* pTexture = pTextureManager->FindTexture( jTextureName->valuestring );
                        if( pTexture == nullptr )
                        {
                            MyFileInfo* pFileInfo = pComponentSystemManager->LoadDataFile( jTextureName->valuestring, sceneIDLoadedFrom, nullptr, false );
                            pTexture = pFileInfo->GetTexture();
                        }

                        // Assign the texture to the component.
                        if( pTexture != nullptr )
                        {
                            TextureDefinition** ppTextureDef = (TextureDefinition**)((char*)pObject + pVar->m_Offset);
                            pTexture->AddRef();
                            SAFE_RELEASE( *ppTextureDef );
                            *ppTextureDef = pTexture;
                        }
                    }
                }
                break;

            case ComponentVariableType_PointerIndirect:
                MyAssert( pVar->m_pSetPointerDescCallBackFunc );
                if( pVar->m_pSetPointerDescCallBackFunc )
                {
                    cJSON* obj = cJSON_GetObjectItem( jComponent, pVar->m_Label );
                    if( obj )
                        (pObjectAsComponent->*pVar->m_pSetPointerDescCallBackFunc)( pVar, obj->valuestring );
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

#if MYFW_USING_WX
void ComponentBase::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentBase::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Unknown component", ObjectListIcon_Component );
    g_pPanelObjectList->SetDragAndDropFunctions( id, ComponentBase::StaticOnDrag, ComponentBase::StaticOnDrop );
}
#endif //MYFW_USING_WX

#if MYFW_EDITOR
bool ComponentBase::IsDivorced(int index)
{
    if( (m_DivorcedVariables & (1 << index)) != 0 )
        return true;

    return false;
}

void ComponentBase::SetDivorced(int index, bool divorced)
{
    MyAssert( index >= 0 && index <= sizeof(unsigned int)*8 );
    if( index < 0 || index > sizeof(unsigned int)*8 )
        return;

    if( divorced )
    {
        m_DivorcedVariables |= (1 << index);
    }
    else
    {
        m_DivorcedVariables &= ~(1 << index);
    }
}

bool ComponentBase::DoesVariableMatchParent(ComponentVariable* pVar, int controlcomponent)
{
    MyAssert( m_pGameObject );
    if( m_pGameObject == 0 )
        return true; // the object has no parent, we say it matches.
    
    GameObject* pGameObject = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pGameObject == 0 )
        return true; // the object has no parent, we say it matches.

    // Found a game object, now find the matching component on it.
    //     Search all components including GameObject properties and transform.
    for( unsigned int i=0; i<pGameObject->GetComponentCountIncludingCore(); i++ )
    {
        ComponentBase* pOtherComponent = pGameObject->GetComponentByIndexIncludingCore( i );

        const char* pThisCompClassName = GetClassname();
        const char* pOtherCompClassName = pOtherComponent->GetClassname();

        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
        {
            size_t offset = pVar->m_Offset;

            switch( pVar->m_Type )
            {
            case ComponentVariableType_Int:
            case ComponentVariableType_Enum:
                return *(int*)((char*)this + offset) == *(int*)((char*)pOtherComponent + offset);

            case ComponentVariableType_UnsignedInt:
            case ComponentVariableType_Flags:
                return *(unsigned int*)((char*)this + offset) == *(unsigned int*)((char*)pOtherComponent + offset);

            //ComponentVariableType_Char,
            //    return *(char*)((char*)this + offset) == *(char*)((char*)pOtherComponent + offset);
            //ComponentVariableType_UnsignedChar,
            //    return *(unsigned char*)((char*)this + offset) == *(unsigned char*)((char*)pOtherComponent + offset);

            case ComponentVariableType_Bool:
                return *(bool*)((char*)this + offset) == *(bool*)((char*)pOtherComponent + offset);

            case ComponentVariableType_Float:
                return *(float*)((char*)this + offset) == *(float*)((char*)pOtherComponent + offset);

            //ComponentVariableType_Double,
            //    return *(double*)((char*)this + offset) == *(double*)((char*)pOtherComponent + offset);

            //ComponentVariableType_ColorFloat,

            case ComponentVariableType_ColorByte:
                if( controlcomponent == 0 )
                {
                    ColorByte* thiscolor = (ColorByte*)((char*)this + offset);
                    ColorByte* parentcolor = (ColorByte*)((char*)pOtherComponent + offset);

                    return *thiscolor == *parentcolor;
                }
                else
                {
                    offset += sizeof(unsigned char)*3; // offset of the alpha in ColorByte
                    return *(unsigned char*)((char*)this + offset) == *(unsigned char*)((char*)pOtherComponent + offset);
                }
                break;

            case ComponentVariableType_Vector2:
            case ComponentVariableType_Vector3:
                offset += controlcomponent*4;
                return *(float*)((char*)this + offset) == *(float*)((char*)pOtherComponent + offset);

            case ComponentVariableType_Vector2Int:
            case ComponentVariableType_Vector3Int:
                offset += controlcomponent*4;
                return *(int*)((char*)this + offset) == *(int*)((char*)pOtherComponent + offset);

            case ComponentVariableType_GameObjectPtr:
                MyAssert( false );
                break;

            case ComponentVariableType_FilePtr:
            case ComponentVariableType_ComponentPtr:
            case ComponentVariableType_MaterialPtr:
            case ComponentVariableType_TexturePtr:
            case ComponentVariableType_SoundCuePtr:
                return *(void**)((char*)this + offset) == *(void**)((char*)pOtherComponent + offset);

            case ComponentVariableType_PointerIndirect:
                return (this->*pVar->m_pGetPointerValueCallBackFunc)( pVar ) == (pOtherComponent->*pVar->m_pGetPointerValueCallBackFunc)( pVar );
                break;

            case ComponentVariableType_NumTypes:
            default:
                MyAssert( false );
                break;
            }
        }
    }

    MyAssert( false ); // shouldn't get here.
    return true; // the object has no parent, we say it matches.
}

void ComponentBase::SyncUndivorcedVariables(ComponentBase* pSourceComponent)
{
    // Sync variables.
    TCPPListHead<ComponentVariable*>* pVariableList = GetComponentVariableList();

    if( pVariableList )
    {
        for( CPPListNode* pNode = pVariableList->GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ComponentVariable* pVar = (ComponentVariable*)pNode;
            MyAssert( pVar );

            pSourceComponent->SyncVariable( this, pVar );
        }
    }
}

void ComponentBase::SyncVariable(ComponentBase* pChildComponent, ComponentVariable* pVar)
{
    if( pChildComponent->IsDivorced( pVar->m_Index ) == true )
        return;

#if MYFW_USING_WX
    // Compare the variable.
    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:
        {
            size_t offset = pVar->m_Offset;

            double oldvalue = *(int*)((char*)pChildComponent + offset);

            // copy the value, call the callback function and update children.
            if( *(int*)((char*)pChildComponent + offset) != *(int*)((char*)this + offset) )
            {
                *(int*)((char*)pChildComponent + offset) = *(int*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, oldvalue, 0 );

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:
        {
            size_t offset = pVar->m_Offset;

            double oldvalue = *(unsigned int*)((char*)pChildComponent + offset);

            // copy the value, call the callback function and update children.
            if( *(unsigned int*)((char*)pChildComponent + offset) != *(unsigned int*)((char*)this + offset) )
            {
                *(unsigned int*)((char*)pChildComponent + offset) = *(unsigned int*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, oldvalue, 0 );

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,

    case ComponentVariableType_Bool:
        {
            size_t offset = pVar->m_Offset;

            double oldvalue = *(bool*)((char*)pChildComponent + offset);

            // copy the value, call the callback function and update children.
            if( *(bool*)((char*)pChildComponent + offset) != *(bool*)((char*)this + offset) )
            {
                *(bool*)((char*)pChildComponent + offset) = *(bool*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, oldvalue, 0 );

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    case ComponentVariableType_Float:
        {
            size_t offset = pVar->m_Offset;
            
            double oldvalue = *(float*)((char*)pChildComponent + offset);

            // copy the value, call the callback function and update children.
            if( *(float*)((char*)pChildComponent + offset) != *(float*)((char*)this + offset) )
            {
                *(float*)((char*)pChildComponent + offset) = *(float*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, oldvalue, 0 );

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,

    case ComponentVariableType_ColorByte:
        for( int component = 0; component < 4; component++ )
        {
            size_t offset = pVar->m_Offset + sizeof(unsigned char)*component;

            double oldvalue = *(unsigned char*)((char*)pChildComponent + offset);

            // copy the value, call the callback function and update children.
            if( *(unsigned char*)((char*)pChildComponent + offset) != *(unsigned char*)((char*)this + offset) )
            {
                *(unsigned char*)((char*)pChildComponent + offset) = *(unsigned char*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, oldvalue, 0 );

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    case ComponentVariableType_Vector2:
    case ComponentVariableType_Vector3:
        for( int component = 0; component < 3; component++ )
        {
            if( pVar->m_Type == ComponentVariableType_Vector2 && component >= 2 )
                continue;

            size_t offset = pVar->m_Offset + component*4;

            double oldvalue = *(float*)((char*)pChildComponent + offset);

            // copy the value, call the callback function and update children.
            if( *(float*)((char*)pChildComponent + offset) != *(float*)((char*)this + offset) )
            {
                *(float*)((char*)pChildComponent + offset) = *(float*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, oldvalue, 0 );

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    case ComponentVariableType_Vector2Int:
    case ComponentVariableType_Vector3Int:
        for( int component = 0; component < 3; component++ )
        {
            if( pVar->m_Type == ComponentVariableType_Vector2 && component >= 2 )
                continue;

            size_t offset = pVar->m_Offset + component*4;
            
            double oldvalue = *(int*)((char*)pChildComponent + offset);

            // copy the value, call the callback function and update children.
            if( *(int*)((char*)pChildComponent + offset) != *(int*)((char*)this + offset) )
            {
                *(int*)((char*)pChildComponent + offset) = *(int*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, oldvalue, 0 );

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    case ComponentVariableType_GameObjectPtr:
        MyAssert( false );
        break;

    case ComponentVariableType_FilePtr:
    case ComponentVariableType_ComponentPtr:
    case ComponentVariableType_MaterialPtr:
    case ComponentVariableType_SoundCuePtr:
        {
            size_t offset = pVar->m_Offset;

            // call the callback function (which will copy the value) and update children.
            MyAssert( pVar->m_pOnValueChangedCallbackFunc );
            if( *(void**)((char*)pChildComponent + offset) != *(void**)((char*)this + offset) )
            {
                ComponentVariableValue newvalue( this, pVar );

                if( pVar->m_pOnValueChangedCallbackFunc )
                {
                    void* oldpointer2 = (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, 0, &newvalue );
                    //MyAssert( oldpointer2 == oldpointer );
                }

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    case ComponentVariableType_PointerIndirect:
        {
            void* childpointer = (pChildComponent->*pVar->m_pGetPointerValueCallBackFunc)( pVar );

            ComponentVariableValue newvalue( this, pVar );
            void* newpointer = (this->*pVar->m_pGetPointerValueCallBackFunc)( pVar );

            if( childpointer != newpointer )
            {
                MyAssert( pVar->m_pSetPointerValueCallBackFunc );
                if( pVar->m_pSetPointerValueCallBackFunc )
                {
                    (pChildComponent->*pVar->m_pSetPointerValueCallBackFunc)( pVar, newpointer );
                }

                MyAssert( pVar->m_pOnValueChangedCallbackFunc );
                if( pVar->m_pOnValueChangedCallbackFunc )
                {
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, false, true, 0, &newvalue );
                }

                pChildComponent->SyncVariableInChildren( pVar );
            }
        }
        break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }
#endif //MYFW_USING_WX
}

void ComponentBase::SyncVariableInChildren(ComponentVariable* pVar)
{
    for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        if( m_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
            continue;

        SceneInfo* pSceneInfo = &m_pComponentSystemManager->m_pSceneInfoMap[i];

        GameObject* pFirstGameObject = pSceneInfo->m_GameObjects.GetHead();
        if( pFirstGameObject )
        {
            SyncVariableInChildrenInGameObjectListWithNewValue( pFirstGameObject, pVar );
        }
    }
}

void ComponentBase::SyncVariableInChildrenInGameObjectListWithNewValue(GameObject* pFirstGameObject, ComponentVariable* pVar)
{
    // find children of this gameobject and change their values as well, if their value matches the old value.
    for( GameObject* pGameObject = pFirstGameObject; pGameObject; pGameObject = pGameObject->GetNext() )
    {
        MyAssert( this->m_pGameObject != 0 );
        if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
        {
            SyncVariableInGameObjectWithNewValue( pGameObject, pVar );
        }

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            SyncVariableInChildrenInGameObjectListWithNewValue( pFirstChild, pVar );
        }
    }
}

void ComponentBase::SyncVariableInGameObjectWithNewValue(GameObject* pGameObject, ComponentVariable* pVar)
{
    MyAssert( this->m_pGameObject != 0 );
    MyAssert( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject );

    {
        // Found a game object, now find the matching component on it.
        //     Search all components including GameObject properties and transform.
        for( unsigned int i=0; i<pGameObject->GetComponentCountIncludingCore(); i++ )
        {
            ComponentBase* pChildComponent = pGameObject->GetComponentByIndexIncludingCore( i );

            const char* pThisCompClassName = GetClassname();
            const char* pOtherCompClassName = pChildComponent->GetClassname();

            // TODO: this will fail if multiple of the same component are on an object.
            if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
            {
                // if this variable in the child component is divorced from us(it's parent), don't update it
                if( pChildComponent->IsDivorced( pVar->m_Index ) )
                    return;

                SyncVariable( pChildComponent, pVar );
            }
        }
    }
}

ComponentBase* ComponentBase::FindMatchingComponentInParent()
{
    // TODO: Search based on m_PrefabComponentID.

    GameObject* pParentGO = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pParentGO == 0 )
        return 0;

    // Found a game object, now find the matching component on it.
    //     Search all components including GameObject properties and transform.
    for( unsigned int i=0; i<pParentGO->GetComponentCountIncludingCore(); i++ )
    {
        ComponentBase* pParentComponent = pParentGO->GetComponentByIndexIncludingCore( i );

        const char* pChildCompClassName = GetClassname();
        const char* pParentCompClassName = pParentComponent->GetClassname();

        if( strcmp( pChildCompClassName, pParentCompClassName ) == 0 )
        {
            return pParentComponent;
        }
    }

    return 0;
}

void ComponentBase::OnValueChangedVariable(int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging, ComponentVariableValue* pNewValue) // StaticOnValueChangedVariable
{
#if MYFW_USING_WX
    ComponentVariable* pVar = FindComponentVariableForControl( controlid );

    // Figure out which component of a multi-component control(e.g. vector3, vector2, colorbyte, etc) this is.
    int controlcomponent = controlid - pVar->m_ControlID;

    OnValueChangedVariable( pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, valuewaschangedbydragging, pNewValue );
#else
    MyAssert( false );
#endif
}

void ComponentBase::OnValueChangedVariable(ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging, ComponentVariableValue* pNewValue) // StaticOnValueChangedVariable
{
    if( pVar )
    {
        void* oldpointer = 0;

        if( pVar->m_pOnValueChangedCallbackFunc )
        {
            oldpointer = (this->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, pNewValue );
        }

        if( m_pGameObject && m_pGameObject->GetGameObjectThisInheritsFrom() )
        {
            // divorce the child value from it's parent, if it no longer matches.
            if( DoesVariableMatchParent( pVar, controlcomponent ) == false ) // returns true if no parent was found.
            {
                // if the variable no longer matches the parent, then divorce it.
                if( directlychanged )
                {
                    if( finishedchanging && IsDivorced( pVar->m_Index ) == false )
                    {
                        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DivorceOrMarryComponentVariable( this, pVar, true ) );
                    }
                }
                else
                {
                    SetDivorced( pVar->m_Index, true );
                    //g_pPanelWatch->ChangeStaticTextFontStyle( pVar->m_ControlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
                    //g_pPanelWatch->ChangeStaticTextBGColor( pVar->m_ControlID, wxColour( 255, 200, 200, 255 ) );
#if MYFW_USING_WX
                    g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX
                }
            }
        }

        UpdateChildrenWithNewValue( false, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, -1, -1, 0 );

        // Deal with multiple selections only if changed by the interface, not if we undo/redo.
        if( directlychanged )
        {
            for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
            {
                if( valuewaschangedbydragging )
                {
                    double newvalue = GetCurrentValueFromVariable( pVar, controlcomponent );
                    double difference = newvalue - oldvalue;

                    if( finishedchanging )
                    {
                        // Apply total change in value, with undo
                        m_MultiSelectedComponents[i]->ChangeValueInNonPointerVariable( pVar, controlcomponent, true, 0, difference );
                    }
                    else
                    {
                        // Apply change in value, no undo
                        m_MultiSelectedComponents[i]->ChangeValueInNonPointerVariable( pVar, controlcomponent, false, difference, difference );
                    }
                }
                else
                {
                    if( finishedchanging )
                    {
                        // Apply original component value, with undo
                        m_MultiSelectedComponents[i]->CopyValueFromOtherComponent( pVar, controlcomponent, this, true );
                    }
                }
            }
        }
    }

    // If this is a prefab, rebuild it.
    if( m_pGameObject->GetPrefabRef()->IsMasterPrefabGameObject() )
    {
        m_pGameObject->GetPrefabRef()->GetPrefab()->RebuildPrefabJSONObjectFromMasterGameObject();
    }
//#endif //MYFW_USING_WX
}


ComponentVariable* ComponentBase::FindComponentVariableByLabel(TCPPListHead<ComponentVariable*>* list, const char* label)
{
    for( CPPListNode* pNode = list->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( strcmp( pVar->m_Label, label ) == 0 )
        {
            return pVar;
        }
    }

    return 0;
}

#if MYFW_USING_WX
void ComponentBase::OnDropVariable(int controlid, int x, int y)
{
    ComponentVariable* pVar = FindComponentVariableForControl( controlid );

    OnDropVariable( pVar, controlid - pVar->m_ControlID, x, y, true );
}
#endif //MYFW_USING_WX

void* ComponentBase::OnDropVariable(ComponentVariable* pVar, int controlComponent, int x, int y, bool addUndoCommand)
{
    if( pVar )
    {
        void* oldPointer = 0;

        if( pVar->m_pOnDropCallbackFunc )
        {
            oldPointer = (this->*pVar->m_pOnDropCallbackFunc)( pVar, addUndoCommand ? true : false, x, y );

            // Create an undo command for this drag/drop action.
            if( addUndoCommand )
            {
                DragAndDropItem* pItem = g_DragAndDropStruct.GetItem( 0 );
                MyAssert( pItem );

                g_pGameCore->GetCommandStack()->Add(
                    MyNew EditorCommand_DragAndDropEvent( this, pVar, 0, -1, -1, pItem->m_Type, pItem->m_Value, oldPointer ) );
            }
        }

        if( m_pGameObject && m_pGameObject->GetGameObjectThisInheritsFrom() )
        {
            // Divorce the child value from it's parent, if it no longer matches.. and it's not already divorced.
            if( addUndoCommand &&
                IsDivorced( pVar->m_Index ) == false &&
                DoesVariableMatchParent( pVar, controlComponent ) == false ) // Returns true if no parent was found.
            {
                // Since the variable no longer matches the parent, then divorce it.
                g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DivorceOrMarryComponentVariable( this, pVar, true ) );
            }
        }

        // OnDropCallback will grab the new value from g_DragAndDropStruct
        UpdateChildrenWithNewValue( true, pVar, controlComponent, false, true, 0, oldPointer, x, y, 0 );

        // deal with multiple selections
        for( unsigned int i=0; i<m_MultiSelectedComponents.size(); i++ )
        {
            UpdateOtherComponentWithNewValue( m_MultiSelectedComponents[i], true, true, true, pVar, controlComponent, true, 0, oldPointer, x, y, 0 );
        }

        return oldPointer;
    }

    return 0;
}

#if MYFW_USING_WX
void ComponentBase::OnRightClickVariable(int controlid)
{
    ComponentVariable* pVar = FindComponentVariableForControl( controlid );
    if( pVar == 0 )
        return;

    // The only right-click options involve this components gameobject having a parent, so quit early if it doesn't
    //    also if there's a callback function, we'll still create the menu.
    if( m_pGameObject->GetGameObjectThisInheritsFrom() == 0 && pVar->m_pOnRightClickCallbackFunc == 0 )
        return;

    wxMenu menu;
    menu.SetClientData( &m_ComponentBaseEventHandlerForComponentVariables );

    m_ComponentBaseEventHandlerForComponentVariables.pComponent = this;
    m_ComponentBaseEventHandlerForComponentVariables.pVar = pVar;
    
    // if this game object inherits from another, right-clicking a variable will offer divorce/marry options.
    if( m_pGameObject->GetGameObjectThisInheritsFrom() )
    {
        if( IsDivorced( pVar->m_Index ) == false )
        {
            menu.Append( RightClick_DivorceVariable, "Divorce value from parent" );
        }
        else
        {
            menu.Append( RightClick_MarryVariable, "Reset value to parent" );
        }
    }

    if( pVar->m_pOnRightClickCallbackFunc )
    {
        (this->*(pVar->m_pOnRightClickCallbackFunc))( pVar, &menu );
    }

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentBaseEventHandlerForComponentVariables::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentBaseEventHandlerForComponentVariables::OnPopupClick(wxEvent &evt)
{
    ComponentBaseEventHandlerForComponentVariables* pEvtHandler = (ComponentBaseEventHandlerForComponentVariables*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    ComponentBase* pComponent = pEvtHandler->pComponent;
    ComponentVariable* pVar = pEvtHandler->pVar;

    int id = evt.GetId();
    switch( id )
    {
    case ComponentBase::RightClick_DivorceVariable:
        {
            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DivorceOrMarryComponentVariable( pComponent, pVar, true ) );
        }
        break;

    case ComponentBase::RightClick_MarryVariable:
        {
            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DivorceOrMarryComponentVariable( pComponent, pVar, false ) );
        }
        break;
    }

    if( pVar->m_pOnPopupClickCallbackFunc )
    {
        (pComponent->*(pVar->m_pOnPopupClickCallbackFunc))( pVar, id );
    }
}

ComponentVariable* ComponentBase::FindComponentVariableForControl(int controlid)
{
    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_ControlID == controlid ||
            (pVar->m_Type == ComponentVariableType_Vector2Int && (pVar->m_ControlID+1 == controlid) ) ||
            (pVar->m_Type == ComponentVariableType_Vector3Int && (pVar->m_ControlID+1 == controlid || pVar->m_ControlID+2 == controlid) ) ||
            (pVar->m_Type == ComponentVariableType_Vector3 && (pVar->m_ControlID+1 == controlid || pVar->m_ControlID+2 == controlid) ) ||
            (pVar->m_Type == ComponentVariableType_Vector2 && (pVar->m_ControlID+1 == controlid) ) ||
            (pVar->m_Type == ComponentVariableType_ColorByte && (pVar->m_ControlID+1 == controlid) )
          )
        {
            return pVar;
        }
    }

    return 0;
}
#endif //MYFW_USING_WX

double ComponentBase::GetCurrentValueFromVariable(ComponentVariable* pVar, int controlcomponent)
{
    double value = 0;

    void* memoryaddr = ((char*)this + pVar->m_Offset);

    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:            value = *(int*)memoryaddr;                              break;
    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:           value = *(unsigned int*)memoryaddr;                     break;
    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,
    case ComponentVariableType_Bool:            value = *(bool*)memoryaddr;                             break;
    case ComponentVariableType_Float:           value = *(float*)memoryaddr;                            break;
    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,
    case ComponentVariableType_ColorByte:                                                               break;
    case ComponentVariableType_Vector2:         value = (*(Vector2*)memoryaddr)[controlcomponent];      break;
    case ComponentVariableType_Vector3:         value = (*(Vector3*)memoryaddr)[controlcomponent];      break;
    case ComponentVariableType_Vector2Int:      value = (*(Vector2Int*)memoryaddr)[controlcomponent];   break;
    case ComponentVariableType_Vector3Int:      value = (*(Vector3Int*)memoryaddr)[controlcomponent];   break;
    case ComponentVariableType_GameObjectPtr:
    case ComponentVariableType_FilePtr:
    case ComponentVariableType_ComponentPtr:
    case ComponentVariableType_MaterialPtr:
    case ComponentVariableType_TexturePtr:
    case ComponentVariableType_SoundCuePtr:
    case ComponentVariableType_PointerIndirect:                                                         break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }

    return value;
}

void ComponentBase::ChangeValueInNonPointerVariable(ComponentVariable* pVar, int controlcomponent, bool addundocommand, double changetoapply, double changeforundo)
{
#if MYFW_USING_IMGUI
    MyAssert( false );
#endif

#if MYFW_USING_WX
    size_t offset = pVar->m_Offset;

    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:
        {
            int oldvalue = *(int*)((char*)this + offset);
            *(int*)((char*)this + offset) = oldvalue + (int)changetoapply;

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
#if MYFW_USING_WX
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_Int, ((char*)this + offset), pVar->m_ControlID, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
#else
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, oldvalue + changeforundo, oldvalue, false ) );
#endif
            }
        }
        break;

    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:
        {
            unsigned int oldvalue = *(unsigned int*)((char*)this + offset);
            *(unsigned int*)((char*)this + offset) = oldvalue + (unsigned int)changetoapply;

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_UnsignedInt, ((char*)this + offset), pVar->m_ControlID, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    //ComponentVariableType_Char,
    //    break;

    //ComponentVariableType_UnsignedChar,
    //    break;

    case ComponentVariableType_Bool:
        {
            // This function should only be used if mouse if dragging a value, which shouldn't be possible with bools.
            MyAssert( false );

            bool oldvalue = *(bool*)((char*)this + offset);
            *(bool*)((char*)this + offset) = (oldvalue + changetoapply) > 0 ? true : false;

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_Bool, ((char*)this + offset), pVar->m_ControlID, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_Float:
        {
            float oldvalue = *(float*)((char*)this + offset);
            *(float*)((char*)this + offset) = oldvalue + (float)changetoapply;

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_Float, ((char*)this + offset), pVar->m_ControlID, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    //ComponentVariableType_Double,
    //    break;

    //ComponentVariableType_ColorFloat,
    //    break;

    case ComponentVariableType_ColorByte:
        {
            MyAssert( false );
            //ColorByte* thiscolor = (ColorByte*)((char*)this + offset);
            //ColorByte* parentcolor = (ColorByte*)((char*)pOtherComponent + offset);

            //ColorByte oldcolor = *thiscolor;
            //*thiscolor = *parentcolor;

            //// store the old color in a local var.
            //// send the pointer to that var via callback in the double.
            //double oldvalue;
            //*(uintptr_t*)&oldvalue = (uintptr_t)&oldcolor;

            //// notify component and it's children that the value changed.
            //OnValueChangedVariable( pVar->m_ControlID, false, true, oldvalue, false, 0 );

            //// TODO: add to undo stack
            //assert( addundocommand == true );
            //if( addundocommand )
            //{
            //}
        }                
        break;

    case ComponentVariableType_Vector2:
        {
            float oldvalue = (*(Vector2*)((char*)this + offset))[controlcomponent];
            (*(Vector2*)((char*)this + offset))[controlcomponent] = oldvalue + (float)changetoapply;

            // Notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID+controlcomponent, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_Float, ((char*)this + offset + sizeof(float)*controlcomponent),
                    pVar->m_ControlID+controlcomponent, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_Vector3:
        {
            float oldvalue = (*(Vector3*)((char*)this + offset))[controlcomponent];
            (*(Vector3*)((char*)this + offset))[controlcomponent] = oldvalue + (float)changetoapply;

            // Notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID+controlcomponent, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_Float, ((char*)this + offset + sizeof(float)*controlcomponent),
                    pVar->m_ControlID+controlcomponent, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_Vector2Int:
        {
            int oldvalue = (*(Vector2Int*)((char*)this + offset))[controlcomponent];
            (*(Vector2Int*)((char*)this + offset))[controlcomponent] = oldvalue + (int)changetoapply;

            // Notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID+controlcomponent, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_Int, ((char*)this + offset + sizeof(int)*controlcomponent),
                    pVar->m_ControlID+controlcomponent, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_Vector3Int:
        {
            int oldvalue = (*(Vector3Int*)((char*)this + offset))[controlcomponent];
            (*(Vector3Int*)((char*)this + offset))[controlcomponent] = oldvalue + (int)changetoapply;

            // Notify component and it's children that the value changed.
            OnValueChangedVariable( pVar->m_ControlID+controlcomponent, false, true, oldvalue, false, 0 );

            if( addundocommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    changeforundo, PanelWatchType_Int, ((char*)this + offset + sizeof(int)*controlcomponent),
                    pVar->m_ControlID+controlcomponent, false,
                    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_GameObjectPtr:
    case ComponentVariableType_FilePtr:
    case ComponentVariableType_MaterialPtr:
    case ComponentVariableType_SoundCuePtr:
    case ComponentVariableType_ComponentPtr:
    case ComponentVariableType_PointerIndirect:
    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }
#endif //MYFW_USING_WX
}

void ComponentBase::CopyValueFromOtherComponent(ComponentVariable* pVar, int controlComponent, ComponentBase* pOtherComponent, bool addUndoCommand)
{
    size_t offset = pVar->m_Offset;

    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            int oldvalue = *(int*)((char*)this + offset);
            int newvalue = *(int*)((char*)pOtherComponent + offset);
            *(int*)((char*)this + offset) = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue - oldvalue, PanelWatchType_Int, ((char*)this + offset), pVar->m_ControlID, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            unsigned int oldvalue = *(unsigned int*)((char*)this + offset);
            unsigned int newvalue = *(unsigned int*)((char*)pOtherComponent + offset);
            *(unsigned int*)((char*)this + offset) = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue - oldvalue, PanelWatchType_UnsignedInt, ((char*)this + offset), pVar->m_ControlID, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    //ComponentVariableType_Char,
    //    break;

    //ComponentVariableType_UnsignedChar,
    //    break;

    case ComponentVariableType_Bool:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            bool oldvalue = *(bool*)((char*)this + offset);
            bool newvalue = *(bool*)((char*)pOtherComponent + offset);
            *(bool*)((char*)this + offset) = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue - oldvalue, PanelWatchType_Bool, ((char*)this + offset), pVar->m_ControlID, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_Float:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            float oldvalue = *(float*)((char*)this + offset);
            float newvalue = *(float*)((char*)pOtherComponent + offset);
            *(float*)((char*)this + offset) = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue - oldvalue, PanelWatchType_Float, ((char*)this + offset), pVar->m_ControlID, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    //ComponentVariableType_Double,
    //    break;

    //ComponentVariableType_ColorFloat,
    //    break;

    case ComponentVariableType_ColorByte:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            ColorByte* thiscolor = (ColorByte*)((char*)this + offset);
            ColorByte* parentcolor = (ColorByte*)((char*)pOtherComponent + offset);

            ColorByte oldcolor = *thiscolor;
            *thiscolor = *parentcolor;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // store the old color in a local var.
            // send the pointer to that var via callback in the double.
            double oldvalue;
            *(uintptr_t*)&oldvalue = (uintptr_t)&oldcolor;

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue, false, 0 );

            // TODO: add to undo stack
            assert( addUndoCommand == true );
            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
            }
        }                
        break;

    case ComponentVariableType_Vector2:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            Vector2 oldvalue = *(Vector2*)((char*)this + offset);
            Vector2 newvalue = *(Vector2*)((char*)pOtherComponent + offset);
            *(Vector2*)((char*)this + offset) = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue.x, false, 0 );
            OnValueChangedVariable( pVar, 1, false, true, oldvalue.y, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue.x - oldvalue.x, PanelWatchType_Float, ((char*)this + offset + 4*0), pVar->m_ControlID+0, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue.y - oldvalue.y, PanelWatchType_Float, ((char*)this + offset + 4*1), pVar->m_ControlID+1, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ), true );
            }
        }
        break;

    case ComponentVariableType_Vector3:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            float oldvalue = (*(Vector3*)((char*)this + offset))[controlComponent];
            float newvalue = (*(Vector3*)((char*)pOtherComponent + offset))[controlComponent];
            (*(Vector3*)((char*)this + offset))[controlComponent] = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // Notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, controlComponent, false, true, oldvalue, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue - oldvalue, PanelWatchType_Float, ((char*)this + offset + 4*controlcomponent),
                //    pVar->m_ControlID+controlcomponent, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
            }
        }
        break;

    case ComponentVariableType_Vector2Int:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            Vector2Int oldvalue = *(Vector2Int*)((char*)this + offset);
            Vector2Int newvalue = *(Vector2Int*)((char*)pOtherComponent + offset);
            *(Vector2Int*)((char*)this + offset) = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue.x, false, 0 );
            OnValueChangedVariable( pVar, 1, false, true, oldvalue.y, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue.x - oldvalue.x, PanelWatchType_Int, ((char*)this + offset + 4*0), pVar->m_ControlID+0, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue.y - oldvalue.y, PanelWatchType_Int, ((char*)this + offset + 4*1), pVar->m_ControlID+1, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ), true );
            }
        }
        break;

    case ComponentVariableType_Vector3Int:
        {
            ComponentVariableValue oldComponentVarValue( this, pVar, this );

            Vector3Int oldvalue = *(Vector3Int*)((char*)this + offset);
            Vector3Int newvalue = *(Vector3Int*)((char*)pOtherComponent + offset);
            *(Vector3Int*)((char*)this + offset) = newvalue;

            ComponentVariableValue newComponentVarValue( this, pVar, this );

            // notify component and it's children that the value changed.
            OnValueChangedVariable( pVar, 0, false, true, oldvalue.x, false, 0 );
            OnValueChangedVariable( pVar, 1, false, true, oldvalue.y, false, 0 );
            OnValueChangedVariable( pVar, 2, false, true, oldvalue.z, false, 0 );

            if( addUndoCommand )
            {
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                    this, pVar, newComponentVarValue, oldComponentVarValue, false, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue.x - oldvalue.x, PanelWatchType_Int, ((char*)this + offset + 4*0), pVar->m_ControlID+0, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ) );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue.y - oldvalue.y, PanelWatchType_Int, ((char*)this + offset + 4*1), pVar->m_ControlID+1, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ), true );
                //g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                //    newvalue.z - oldvalue.z, PanelWatchType_Int, ((char*)this + offset + 4*2), pVar->m_ControlID+2, false,
                //    ComponentBase::StaticOnValueChangedVariable, this ), true );
            }
        }
        break;

    // Pointers types needs to add to undo manually in their OnValueChanged callbacks.
    case ComponentVariableType_GameObjectPtr:
        {
            // OnDrop will add an undo command, so this method shouldn't be called for this ComponentVariableType.
            assert( addUndoCommand == true );

            g_DragAndDropStruct.Clear();
            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
            g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, *(MyFileObject**)((char*)pOtherComponent + offset) );
            OnDropVariable( pVar, 0, 0, 0 );
        }
        break;

    case ComponentVariableType_FilePtr:
        {
            // OnDrop will add an undo command, so this method shouldn't be called for this ComponentVariableType.
            assert( addUndoCommand == true );

            g_DragAndDropStruct.Clear();
            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
            g_DragAndDropStruct.Add( DragAndDropType_FileObjectPointer, *(MyFileObject**)((char*)pOtherComponent + offset) );
            OnDropVariable( pVar, 0, 0, 0 );
        }
        break;

    case ComponentVariableType_MaterialPtr:
        {
            // OnDrop will add an undo command, so this method shouldn't be called for this ComponentVariableType.
            assert( addUndoCommand == true );

            g_DragAndDropStruct.Clear();
            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
            g_DragAndDropStruct.Add( DragAndDropType_MaterialDefinitionPointer, *(void**)((char*)pOtherComponent + offset) );
            OnDropVariable( pVar, 0, 0, 0 );
        }
        break;

    case ComponentVariableType_TexturePtr:
        {
            // OnDrop will add an undo command, so this method shouldn't be called for this ComponentVariableType.
            assert( addUndoCommand == true );

            g_DragAndDropStruct.Clear();
            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
            g_DragAndDropStruct.Add( DragAndDropType_TextureDefinitionPointer, *(void**)((char*)pOtherComponent + offset) );
            OnDropVariable( pVar, 0, 0, 0 );
        }
        break;

    case ComponentVariableType_SoundCuePtr:
        {
            // OnDrop will add an undo command, so this method shouldn't be called for this ComponentVariableType.
            assert( addUndoCommand == true );

            void* newvalue = *(void**)((char*)pOtherComponent + offset);

            g_DragAndDropStruct.Clear();
            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
            g_DragAndDropStruct.Add( DragAndDropType_SoundCuePointer, newvalue );
            OnDropVariable( pVar, 0, 0, 0 );
        }
        break;

    case ComponentVariableType_ComponentPtr:
        {
            // OnDrop will add an undo command, so this method shouldn't be called for this ComponentVariableType.
            assert( addUndoCommand == true );

            g_DragAndDropStruct.Clear();
            g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
            g_DragAndDropStruct.Add( DragAndDropType_ComponentPointer, *(MyFileObject**)((char*)pOtherComponent + offset) );
            OnDropVariable( pVar, 0, 0, 0 );
        }
        break;

    case ComponentVariableType_PointerIndirect:
        {
            // Will add an undo command, so this method shouldn't be called for this ComponentVariableType.
            assert( addUndoCommand == true );

            void* pParentValue = (pOtherComponent->*pVar->m_pGetPointerValueCallBackFunc)( pVar );
            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ComponentVariableIndirectPointerChanged( this, pVar, pParentValue ) );
        }
        break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }
}

void ComponentBase::UpdateChildrenWithNewValue(bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer)
{
#if 0 //MYFW_USING_WX
    typedef std::map<int, SceneInfo>::iterator it_type;
    for( it_type iterator = m_pComponentSystemManager->m_pSceneInfoMap.begin(); iterator != m_pComponentSystemManager->m_pSceneInfoMap.end(); )
    {
        SceneID sceneid = iterator->first;
        SceneInfo* pSceneInfo = &iterator->second;
#else
    for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
            continue;

        SceneInfo* pSceneInfo = &g_pComponentSystemManager->m_pSceneInfoMap[i];
#endif //MYFW_USING_WX

        if( pSceneInfo->m_GameObjects.GetHead() )
        {
            GameObject* pFirstGameObject = pSceneInfo->m_GameObjects.GetHead();
            UpdateChildrenInGameObjectListWithNewValue( pFirstGameObject, fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
    }
}

void ComponentBase::UpdateChildrenInGameObjectListWithNewValue(GameObject* pFirstGameObject, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer)
{
    // find children of this gameobject and change their values as well, if their value matches the old value.
    for( GameObject* pGameObject = pFirstGameObject; pGameObject; pGameObject = pGameObject->GetNext() )
    {
        MyAssert( this->m_pGameObject != 0 );
        if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
        {
            UpdateGameObjectWithNewValue( pGameObject, fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            UpdateChildrenInGameObjectListWithNewValue( pFirstChild, fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
    }
}

void ComponentBase::UpdateGameObjectWithNewValue(GameObject* pGameObject, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer)
{
    MyAssert( this->m_pGameObject != 0 );
    MyAssert( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject );

    {
        ComponentBase* pMatchingComponent = 0;

        if( pGameObject->IsPrefabInstance() && m_PrefabComponentID != 0 )
        {
            // This GameObject is an instance of a prefab.
            // Search for the component using the PrefabComponentID.
            pMatchingComponent = pGameObject->FindComponentByPrefabComponentID( m_PrefabComponentID );
        }
        else
        {
            // This GameObject is *not* an instance of a prefab, it's using the direct inheritance system.
            // TODO: Take direct inheritance out? or have it use PrefabComponentID as well.

            // Found a game object, now find the matching component on it.
            // Search all components including GameObject properties and transform.
            for( unsigned int i=0; i<pGameObject->GetComponentCountIncludingCore(); i++ )
            {
                ComponentBase* pChildComponent = pGameObject->GetComponentByIndexIncludingCore( i );

                const char* pThisCompClassName = GetClassname();
                const char* pOtherCompClassName = pChildComponent->GetClassname();

                // TODO: this will fail if multiple of the same component are on an object.
                if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
                {
                    pMatchingComponent = pChildComponent;
                }
            }
        }

        if( pMatchingComponent )
        {
            // if this variable in the child component is divorced from us(it's parent), don't update it
            if( pMatchingComponent->IsDivorced( pVar->m_Index ) )
                return;

            UpdateOtherComponentWithNewValue( pMatchingComponent, directlychanged, false, fromdraganddrop, pVar, controlcomponent, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
    }
}

void ComponentBase::UpdateOtherComponentWithNewValue(ComponentBase* pComponent, bool directlychanged, bool ignoreDivorceStatus, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer)
{
    ComponentBase* pChildComponent = pComponent;

    bool updateValue = false;
    if( ignoreDivorceStatus || pChildComponent->IsDivorced( pVar->m_Index ) == false )
        updateValue = true;

    if( updateValue == false )
        return;

    // Found the matching component, now compare the variable.
    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:
        MyAssert( fromdraganddrop == false ); // not drag/dropping these types ATM.

        if( fromdraganddrop == false )
        {
            size_t offset = pVar->m_Offset;

            // copy the value, call the callback function and update children.
            *(int*)((char*)pChildComponent + offset) = *(int*)((char*)this + offset);
            if( pVar->m_pOnValueChangedCallbackFunc )
                (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
        break;

    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:
        MyAssert( fromdraganddrop == false ); // not drag/dropping these types ATM.

        if( fromdraganddrop == false )
        {
            size_t offset = pVar->m_Offset;

            // copy the value, call the callback function and update children.
            *(unsigned int*)((char*)pChildComponent + offset) = *(unsigned int*)((char*)this + offset);
            if( pVar->m_pOnValueChangedCallbackFunc )
                (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
        break;

    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,

    case ComponentVariableType_Bool:
        MyAssert( fromdraganddrop == false ); // not drag/dropping these types ATM.

        if( fromdraganddrop == false )
        {
            size_t offset = pVar->m_Offset;

            // copy the value, call the callback function and update children.
            *(bool*)((char*)pChildComponent + offset) = *(bool*)((char*)this + offset);
            if( pVar->m_pOnValueChangedCallbackFunc )
                (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
        break;

    case ComponentVariableType_Float:
        MyAssert( fromdraganddrop == false ); // not drag/dropping these types ATM.

        if( fromdraganddrop == false )
        {
            size_t offset = pVar->m_Offset;
            
            // copy the value, call the callback function and update children.
            *(float*)((char*)pChildComponent + offset) = *(float*)((char*)this + offset);
            if( pVar->m_pOnValueChangedCallbackFunc )
                (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
        break;

    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,

    case ComponentVariableType_ColorByte:
        MyAssert( fromdraganddrop == false ); // not drag/dropping these types ATM.

        if( fromdraganddrop == false )
        {
            if( controlcomponent == 0 ) // color picker box
            {
                if( oldvalue != 0 )
                {
                    size_t offset = pVar->m_Offset;

                    ColorByte* oldcolor = (ColorByte*)*(size_t*)&oldvalue;
                    ColorByte* childcolor = (ColorByte*)((char*)pChildComponent + offset);
                    
                    // copy the value, call the callback function and update children.
                    ColorByte* newcolor = (ColorByte*)((char*)this + offset);
                    *childcolor = *newcolor;
                    if( pVar->m_pOnValueChangedCallbackFunc )
                        (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );
                }
            }
            else
            {
                MyAssert( controlcomponent == 1 ); // alpha control is next to the color picker box

                size_t offset = pVar->m_Offset + sizeof(unsigned char)*3; // offset of the alpha in ColorByte

                *(unsigned char*)((char*)pChildComponent + offset) = *(unsigned char*)((char*)this + offset);
                if( pVar->m_pOnValueChangedCallbackFunc )
                    (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );
            }

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
        break;

    case ComponentVariableType_Vector2:
    case ComponentVariableType_Vector3:
        MyAssert( fromdraganddrop == false ); // not drag/dropping these types ATM.

        if( fromdraganddrop == false )
        {
            size_t offset = pVar->m_Offset + controlcomponent*4;

            // copy the value, call the callback function and update children.
            *(float*)((char*)pChildComponent + offset) = *(float*)((char*)this + offset);
            if( pVar->m_pOnValueChangedCallbackFunc )
                (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
        break;

    case ComponentVariableType_Vector2Int:
    case ComponentVariableType_Vector3Int:
        MyAssert( fromdraganddrop == false ); // not drag/dropping these types ATM.

        if( fromdraganddrop == false )
        {
            size_t offset = pVar->m_Offset + controlcomponent*4;
            
            // copy the value, call the callback function and update children.
            *(int*)((char*)pChildComponent + offset) = *(int*)((char*)this + offset);
            if( pVar->m_pOnValueChangedCallbackFunc )
                (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, 0 );

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, newpointer );
        }
        break;

    case ComponentVariableType_GameObjectPtr:
        MyAssert( false );
        break;

    case ComponentVariableType_FilePtr:
    case ComponentVariableType_ComponentPtr:
    case ComponentVariableType_MaterialPtr:
    case ComponentVariableType_TexturePtr:
    case ComponentVariableType_SoundCuePtr:
        {
            if( fromdraganddrop )
            {
                size_t offset = pVar->m_Offset;

                // OnDropCallback will grab the new value from g_DragAndDropStruct
                MyAssert( pVar->m_pOnDropCallbackFunc );
                if( pVar->m_pOnDropCallbackFunc )
                {
                    void* oldpointer2 = (pChildComponent->*pVar->m_pOnDropCallbackFunc)( pVar, true, x, y );

                    // assert should only trip if child didn't have same value that the parent had
                    //     which shouldn't happen since values aren't divorced.
                    // could happen since divorced flags are saved in .scene files and not verified on load
                    // nothing bad will happen if assert trips, other than child value getting overwritten unexpectedly.
                    // TODO: temp removed since will also trip if parent/child are multiselected and pointer changes.
                    //MyAssert( oldpointer2 == oldpointer );
                }
            }
            else
            {
                size_t offset = pVar->m_Offset;
                
                // call the callback function and update children.
                MyAssert( pVar->m_pOnValueChangedCallbackFunc );
                if( pVar->m_pOnValueChangedCallbackFunc )
                {
                    //void* newpointer = *(void**)((char*)this + offset);
                    ComponentVariableValue newpointer( this, pVar, this );

                    void* oldpointer2 = (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, &newpointer );
                    //MyAssert( oldpointer2 == oldpointer );
                }
            }

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, &newpointer );
        }
        break;

    case ComponentVariableType_PointerIndirect:
        {
            if( fromdraganddrop )
            {
                size_t offset = pVar->m_Offset;

                // OnDropCallback will grab the new value from g_DragAndDropStruct
                MyAssert( pVar->m_pOnDropCallbackFunc );
                if( pVar->m_pOnDropCallbackFunc )
                {
                    void* oldpointer2 = (pChildComponent->*pVar->m_pOnDropCallbackFunc)( pVar, true, x, y );
                    //MyAssert( oldpointer2 == oldpointer );
                }
            }
            else if( newpointer )
            {
                MyAssert( pVar->m_pSetPointerValueCallBackFunc );
                if( pVar->m_pSetPointerValueCallBackFunc )
                {
                    (pChildComponent->*pVar->m_pSetPointerValueCallBackFunc)( pVar, newpointer );
                }
            }
            else
            {
                MyAssert( pVar->m_pOnValueChangedCallbackFunc );
                if( pVar->m_pOnValueChangedCallbackFunc )
                {
                    ComponentVariableValue newpointer( this, pVar, this );

                    void* oldpointer2 = (pChildComponent->*pVar->m_pOnValueChangedCallbackFunc)( pVar, directlychanged, finishedchanging, oldvalue, &newpointer );
                    //MyAssert( oldpointer2 == oldpointer );
                }
            }

            pChildComponent->UpdateChildrenWithNewValue( fromdraganddrop, pVar, controlcomponent, directlychanged, finishedchanging, oldvalue, oldpointer, x, y, &newpointer );
        }
        break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }
}

#if MYFW_USING_WX
void ComponentBase::OnComponentTitleLabelClicked(int id, bool finishedchanging)
{
    if( id != -1 && id == m_ControlID_ComponentTitleLabel )
    {
        if( m_pPanelWatchBlockVisible )
        {
            *m_pPanelWatchBlockVisible = !(*m_pPanelWatchBlockVisible);
            g_pPanelWatch->SetNeedsRefresh();
        }
    }
}

void ComponentBase::OnLeftClick(unsigned int count, bool clear)
{
    if( count <= 1 )
        m_MultiSelectedComponents.clear();

    // select this Component in the editor window.
    g_pEngineCore->GetEditorState()->m_pSelectedComponents.push_back( this );

    if( clear )
        g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->SetObjectBeingWatched( this );

    FillPropertiesWindow( clear, true, true );
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
    pMenu->Append( RightClick_DeleteComponent, "Delete Component" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentBase::OnPopupClick );
}

void ComponentBase::OnPopupClick(wxEvent &evt)
{
    //ComponentBase* pComponent = (ComponentBase*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    int id = evt.GetId();

    OnRightClickAction( id );
}

void ComponentBase::OnDrag()
{
    g_DragAndDropStruct.Add( DragAndDropType_ComponentPointer, this );
}

void ComponentBase::OnDrop(int controlid, int x, int y)
{
}
#endif //MYFW_USING_WX

bool ComponentBase::IsReferencingFile(MyFileObject* pFile)
{
    if( GetComponentVariableList() == 0 )
        return false;

    for( CPPListNode* pNode = GetComponentVariableList()->GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        //if( pVar->m_Type == ComponentVariableType_GameObjectPtr )
        //if( pVar->m_Type == ComponentVariableType_ComponentPtr )
        if( pVar->m_Type == ComponentVariableType_FilePtr )
        {
            MyFileObject* pFileFromComponent = (MyFileObject*)(*(void**)((char*)this + pVar->m_Offset));
            if( pFileFromComponent && pFileFromComponent == pFile )
                return true;
        }
        else if( pVar->m_Type == ComponentVariableType_MaterialPtr )
        {
            MaterialDefinition* pMaterial = (MaterialDefinition*)(*(void**)((char*)this + pVar->m_Offset));
            if( pMaterial && pMaterial->GetFile() == pFile )
                return true;
        }
        else if( pVar->m_Type == ComponentVariableType_SoundCuePtr )
        {
            SoundCue* pSoundCue = (SoundCue*)(*(void**)((char*)this + pVar->m_Offset));
            if( pSoundCue && pSoundCue->GetFile() == pFile )
                return true;
        }
        else if( pVar->m_Type == ComponentVariableType_PointerIndirect )
        {
            void* ptr = (this->*pVar->m_pGetPointerValueCallBackFunc)( pVar );
            if( ptr == pFile )
                return true;
        }
    }

    return false;
}

void ComponentBase::OnRightClickAction(int action)
{
    if( action == RightClick_DeleteComponent )
    {
        EditorState* pEditorState = m_pEngineCore->GetEditorState();

        // if anything is still selected, delete it/them.
        if( pEditorState->m_pSelectedComponents.size() > 0 )
        {
            g_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteComponents( pEditorState->m_pSelectedComponents ) );
        }

        g_pEngineCore->GetEditorState()->ClearSelectedComponents();
    }
}
#endif //MYFW_EDITOR
