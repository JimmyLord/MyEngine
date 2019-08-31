//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"

#include "ComponentScriptBase.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#endif

ComponentScriptBase::ComponentScriptBase(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentUpdateable( pEngineCore, pComponentSystemManager )
{
    m_ExposedVars.AllocateObjects( MAX_EXPOSED_VARS ); // Hard coded nonsense for now, max of 4 exposed vars in a script.
}

ComponentScriptBase::~ComponentScriptBase()
{
}

cJSON* ComponentScriptBase::ExportExposedVariablesAsJSONObject()
{
    cJSON* jExposedVarArray = cJSON_CreateArray();

    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        cJSON* jExposedVar = cJSON_CreateObject();
        cJSON_AddItemToArray( jExposedVarArray, jExposedVar );

        cJSON_AddStringToObject( jExposedVar, "Name", pVar->name.c_str() );
        cJSON_AddNumberToObject( jExposedVar, "Type", (int)pVar->type );

        if( pVar->type == ExposedVariableType::Float )
        {
            cJSON_AddNumberToObject( jExposedVar, "Value", pVar->valueDouble );
        }
        if( pVar->type == ExposedVariableType::Bool )
        {
            cJSON_AddNumberToObject( jExposedVar, "Value", pVar->valueBool );
        }
        if( pVar->type == ExposedVariableType::Vector3 )
        {
            cJSONExt_AddFloatArrayToObject( jExposedVar, "Value", &pVar->valueVec3.x, 3 );
        }
        else if( pVar->type == ExposedVariableType::GameObject && pVar->pointer )
        {
            cJSON* gameobjectref = static_cast<GameObject*>( pVar->pointer )->ExportReferenceAsJSONObject( m_SceneIDLoadedFrom );
            cJSON_AddItemToObject( jExposedVar, "Value", gameobjectref );

            // TODO: Find a way to uniquely identify a game object...
            //cJSON_AddStringToObject( jExposedVar, "Value", static_cast<GameObject*>( pVar->pointer )->GetName() );
        }

        if( pVar->divorced )
            cJSON_AddNumberToObject( jExposedVar, "Divorced", pVar->divorced );
    }

    return jExposedVarArray;
}

void ComponentScriptBase::ImportExposedVariablesFromJSONObject(cJSON* jExposedVarArray)
{
    MyAssert( jExposedVarArray );

    int numVars = cJSON_GetArraySize( jExposedVarArray );
    for( int i=0; i<numVars; i++ )
    {
        cJSON* jsonvar = cJSON_GetArrayItem( jExposedVarArray, i );

        cJSON* obj = cJSON_GetObjectItem( jsonvar, "Name" );
        MyAssert( obj );
        if( obj == nullptr )
            continue;

        // By name, check if the variable is already in our list.
        ExposedVariableDesc* pVar = nullptr;
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            MyAssert( m_ExposedVars[i] );
            if( m_ExposedVars[i]->name == obj->valuestring )
            {
                pVar = m_ExposedVars[i];
                break;
            }
        }

        // If not, create and add it.
        if( pVar == nullptr )
        {
            pVar = MyNew ExposedVariableDesc();
            m_ExposedVars.Add( pVar );
        }

        pVar->Reset();

        pVar->name = obj->valuestring;
        cJSONExt_GetInt( jsonvar, "Type", (int*)&pVar->type );

        if( pVar->type == ExposedVariableType::Float )
        {
            cJSONExt_GetDouble( jsonvar, "Value", &pVar->valueDouble );
        }
        if( pVar->type == ExposedVariableType::Bool )
        {
            cJSONExt_GetBool( jsonvar, "Value", &pVar->valueBool );
        }
        if( pVar->type == ExposedVariableType::Vector3 )
        {
            cJSONExt_GetFloatArray( jsonvar, "Value", &pVar->valueVec3.x, 3 );
        }
        else if( pVar->type == ExposedVariableType::GameObject )
        {
            cJSON* obj = cJSON_GetObjectItem( jsonvar, "Value" );
            if( obj )
            {
                pVar->pointer = m_pComponentSystemManager->FindGameObjectByJSONRef( obj, m_pGameObject->GetSceneID(), false );

                // TODO: Handle cases where the Scene containing the GameObject referenced isn't loaded.
                //MyAssert( pVar->pointer != nullptr );
                if( pVar->pointer )
                {
                    static_cast<GameObject*>( pVar->pointer )->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                }
                else
                {
                    LOGError( LOGTag, "LuaScript component on '%s' lost reference to GameObject.\n", m_pGameObject->GetName() );
                }
            }
        }

        cJSONExt_GetBool( jsonvar, "Divorced", &pVar->divorced );
    }
}

void ComponentScriptBase::AddExposedVariablesToInterface()
{
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        switch( pVar->type )
        {
        case ExposedVariableType::Unused:
            MyAssert( false );
            break;

        case ExposedVariableType::Float:
            {
                float tempFloat = (float)pVar->valueDouble;
                bool modified = ImGui::DragFloat( pVar->name.c_str(), &tempFloat, 0.1f );
                if( modified )
                {
                    TestForExposedVariableModificationAndCreateUndoCommand( this, ImGuiExt::GetActiveItemId(), modified, pVar, tempFloat );
                }
            }
            break;

        case ExposedVariableType::Bool:
            {
                ImGui::Text( "(TODO) Bool: %s", pVar->name.c_str() );
                //id = g_pPanelWatch->AddBool( pVar->name.c_str(), &pVar->valueBool, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;

        case ExposedVariableType::Vector3:
            {
                ImGui::Text( "(TODO) Vector3: %s", pVar->name.c_str() );
                //id = g_pPanelWatch->AddVector3( pVar->name.c_str(), (Vector3*)&pVar->valueVec3, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;

        case ExposedVariableType::GameObject:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                GameObject* pGameObject = static_cast<GameObject*>( pVar->pointer );

                const char* pDesc = "none";
                if( pGameObject )
                {
                    pDesc = pGameObject->GetName();
                }

                float width = ImGui::GetWindowWidth() * 0.65f;
                if( ImGui::Button( pDesc, ImVec2( width, 0 ) ) )
                {
                    // TODO: Pop up a GameObject picker window.
                }

                if( ImGui::BeginDragDropTarget() )
                {
                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "GameObject" ) )
                    {
                        // Set the new value.
                        GameObject* pDroppedGameObject = (GameObject*)*(void**)payload->Data;
                        m_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_ExposedVariablePointerChanged(
                            pDroppedGameObject, pVar, ComponentScriptBase::StaticOnExposedVarValueChanged, this ), true );
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::Text( pVar->name.c_str() );

                ImGui::EndGroup();

                // Right-click menu on group.
                ImGui::PushID( pVar );
                if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                {
                    //// Set color to default, since it might be set to divorced color.
                    //Vector4 color = m_pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                    //ImGui::PushStyleColor( ImGuiCol_Text, color );

                    if( ImGui::MenuItem( "Clear GameObject" ) )
                    {
                        // Set the new value.
                        GameObject* pNewGameObject = nullptr;
                        m_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_ExposedVariablePointerChanged(
                            pNewGameObject, pVar, ComponentScriptBase::StaticOnExposedVarValueChanged, this ), true );
                    }

                    //ImGui::PopStyleColor( 1 );

                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            break;
        }

        if( pVar->divorced )
        {
        }
    }
}

void ComponentScriptBase::CopyExposedVariablesFromOtherComponent(const ComponentScriptBase& other)
{
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];
        ExposedVariableDesc* pOtherVar = nullptr;// = other.m_ExposedVars[i];            

        // Find the first variable in the other object with the same name.
        for( unsigned int oi=0; oi<m_ExposedVars.Count(); oi++ )
        {
            if( pVar->name == other.m_ExposedVars[oi]->name )
            {
                pOtherVar = other.m_ExposedVars[oi];
                break;
            }
        }

        if( pOtherVar != nullptr )
        {
            if( pVar->type == ExposedVariableType::Float )
            {
                pVar->valueDouble = pOtherVar->valueDouble;
            }

            if( pVar->type == ExposedVariableType::Bool )
            {
                pVar->valueBool = pOtherVar->valueBool;
            }

            if( pVar->type == ExposedVariableType::Vector3 )
            {
                for( int i=0; i<3; i++ )
                    pVar->valueVec3[0] = pOtherVar->valueVec3[0];
            }

            if( pVar->type == ExposedVariableType::GameObject )
            {
                pVar->pointer = pOtherVar->pointer;

                if( pVar->pointer )
                    static_cast<GameObject*>( pVar->pointer )->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
            }
        }
    }
}

// Static.
void ComponentScriptBase::TestForExposedVariableModificationAndCreateUndoCommand(ComponentScriptBase* pComponent, ImGuiID id, bool modified, ExposedVariableDesc* pVar, double newValue)
{
    MyAssert( pComponent != nullptr );

    // If the id passed in is different than the last known value, then assume a new control was selected.
    if( id != pComponent->m_ImGuiControlIDForCurrentlySelectedVariable )
    {
        // If a new control was selected, store the starting value and start a new undo chain.
        pComponent->m_ValueWhenControlSelected = pVar->valueDouble;
        pComponent->m_LinkNextUndoCommandToPrevious = false;
        pComponent->m_ImGuiControlIDForCurrentlySelectedVariable = id;
    }

    // If the control returned true to indicate it was modified, then create an undo command.
    if( modified && id != 0 )
    {
        MyAssert( id == pComponent->m_ImGuiControlIDForCurrentlySelectedVariable );

        // Add an undo action.
        EditorCommand* pCommand = MyNew EditorCommand_ExposedVariableFloatChanged(
            newValue, pVar, ComponentScriptBase::StaticOnExposedVarValueChanged, pComponent );

        pComponent->GetComponentSystemManager()->GetEngineCore()->GetCommandStack()->Do( pCommand, pComponent->m_LinkNextUndoCommandToPrevious );

        // Link the next undo command to this one.
        // TODO: Since we're passing in the starting value,
        //       we can actually replace the old command rather than link to it.
        pComponent->m_LinkNextUndoCommandToPrevious = true;
    }
}

bool ComponentScriptBase::DoesExposedVariableMatchParent(ExposedVariableDesc* pVar)
{
    MyAssert( m_pGameObject );
    if( m_pGameObject == nullptr )
        return true; // The object has no parent, we say it matches.

    GameObject* pGameObject = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pGameObject == nullptr )
        return true; // The object has no parent, we say it matches.

                     // Found a game object, now find the matching component on it.
    for( unsigned int i=0; i<pGameObject->GetComponentCount(); i++ )
    {
        ComponentBase* pOtherComponent = pGameObject->GetComponentByIndex( i );

        const char* pThisCompClassName = GetClassname();
        const char* pOtherCompClassName = pOtherComponent->GetClassname();

        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
        {
            ComponentScriptBase* pOtherScript = static_cast<ComponentScriptBase*>( pOtherComponent );

            // Find children of this gameobject and change their vars if needed.
            for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
            {
                ExposedVariableDesc* pOtherVar = pOtherScript->m_ExposedVars[varindex];
                MyAssert( pOtherVar );

                if( pVar->name == pOtherVar->name )
                {
                    switch( pVar->type )
                    {
                    case ExposedVariableType::Float:
                        return pVar->valueDouble == pOtherVar->valueDouble;

                    case ExposedVariableType::Bool:
                        return pVar->valueBool == pOtherVar->valueBool;

                    case ExposedVariableType::Vector3:
                        return pVar->valueVec3 == pOtherVar->valueVec3;

                    case ExposedVariableType::GameObject:
                        return pVar->pointer == pOtherVar->pointer;

                    case ExposedVariableType::Unused:
                    default:
                        MyAssert( false );
                        break;
                    }
                }
            }
        }
    }

    MyAssert( false ); // Shouldn't get here.
    return true; // The object has no parent, we say it matches.
}

void ComponentScriptBase::UpdateChildrenWithNewValue(ExposedVariableDesc* pVar, bool finishedChanging, double oldValue, void* oldPointer)
{
    MyAssert( pVar );

    // Find children of this gameobject and change their vars if needed.
    for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
    {
        ExposedVariableDesc* pOtherVar = m_ExposedVars[varindex];
        MyAssert( pOtherVar );

        if( pVar->name == pOtherVar->name )
        {
            for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
            {
                if( m_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                    continue;

                SceneInfo* pSceneInfo = &m_pComponentSystemManager->m_pSceneInfoMap[i];

                if( pSceneInfo->m_GameObjects.GetHead() )
                {
                    GameObject* first = pSceneInfo->m_GameObjects.GetHead();
                    UpdateChildrenInGameObjectListWithNewValue( pVar, varindex, first, finishedChanging, oldValue, oldPointer );
                } 
            }
        }
    }
}

void ComponentScriptBase::UpdateChildrenInGameObjectListWithNewValue(ExposedVariableDesc* pVar, unsigned int varindex, GameObject* first, bool finishedChanging, double oldValue, void* oldPointer)
{
    // Find children of this gameobject and change their values as well, if their value matches the old value.
    for( GameObject* pGameObject = first; pGameObject; pGameObject = pGameObject->GetNext() )
    {
        if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
        {
            UpdateChildGameObjectWithNewValue( pVar, varindex, pGameObject, finishedChanging, oldValue, oldPointer );
        }

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            UpdateChildrenInGameObjectListWithNewValue( pVar, varindex, pFirstChild, finishedChanging, oldValue, oldPointer );
        }
    }
}

void ComponentScriptBase::UpdateChildGameObjectWithNewValue(ExposedVariableDesc* pVar, unsigned int varIndex, GameObject* pChildGameObject, bool finishedChanging, double oldValue, void* oldPointer)
{
    if( pChildGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
    {
        // Found a game object, now find the matching component on it.
        for( unsigned int i=0; i<pChildGameObject->GetComponentCount(); i++ )
        {
            ComponentScriptBase* pChildScript = static_cast<ComponentScriptBase*>( pChildGameObject->GetComponentByIndex( i ) );

            const char* pThisCompClassName = GetClassname();
            const char* pChildCompClassName = pChildScript->GetClassname();

            // TODO: This will fail if multiple of the same component are on an object.
            if( strcmp( pThisCompClassName, pChildCompClassName ) == 0 )
            {
                // It's possible the variables are in a different order, so find the correct variable by name.
                ExposedVariableDesc* pChildVar = nullptr;

                // Find the first variable in the other object with the same name.
                for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
                {
                    if( pChildScript->m_ExposedVars.Count() > i &&
                        m_ExposedVars[varIndex]->name == pChildScript->m_ExposedVars[i]->name )
                    {
                        pChildVar = pChildScript->m_ExposedVars[i];
                        break;
                    }
                }

                if( pChildVar )
                {
                    // Found the matching component, now compare the variable.
                    if( pChildVar->type == ExposedVariableType::Float ||
                        pChildVar->type == ExposedVariableType::Bool )
                    {
                        if( fequal( pChildVar->valueDouble, oldValue ) )
                        {
                            pChildVar->valueDouble = pVar->valueDouble;
                            //pChildScript->OnExposedVarValueChanged( controlid, finishedchanging, oldvalue );

                            pChildScript->ProgramVariables( true );
                            pChildScript->UpdateChildrenWithNewValue( pChildVar, finishedChanging, oldValue, oldPointer );
                        }
                    }

                    if( pChildVar->type == ExposedVariableType::Vector3 )
                    {
                        MyAssert( false );
                        //if( fequal( pChildVar->valueVec3[0], oldvalue ) )
                        //{
                        //    pChildVar->valueDouble = pVar->valueDouble;
                        //    pChildScript->OnValueChanged( controlid, finishedchanging, oldvalue );

                        //    pChildScript->ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                        //    pChildScript->UpdateChildrenWithNewValue( controlid, finishedChanging, oldValue, oldPointer );
                        //}
                    }

                    if( pVar->type == ExposedVariableType::GameObject )
                    {
                        if( pChildVar->pointer == oldPointer )
                        {
                            pChildVar->pointer = pVar->pointer;
                            if( pVar->pointer )
                                static_cast<GameObject*>( pVar->pointer )->RegisterOnDeleteCallback( pChildScript, StaticOnGameObjectDeleted );

                            pChildScript->ProgramVariables( true );
                            pChildScript->UpdateChildrenWithNewValue( pChildVar, finishedChanging, oldValue, oldPointer );
                        }
                    }
                }
            }
        }
    }
}

void ComponentScriptBase::CopyExposedVarValueFromParent(ExposedVariableDesc* pVar)
{
    MyAssert( m_pGameObject );
    MyAssert( m_pGameObject->GetGameObjectThisInheritsFrom() );

    GameObject* pParentGO = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pParentGO == nullptr )
        return;

    // Found a game object, now find the matching component on it.
    for( unsigned int i=0; i<pParentGO->GetComponentCount(); i++ )
    {
        ComponentBase* pOtherComponent = pParentGO->GetComponentByIndex( i );

        const char* pThisCompClassName = GetClassname();
        const char* pOtherCompClassName = pOtherComponent->GetClassname();

        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
        {
            ComponentScriptBase* pOtherScript = static_cast<ComponentScriptBase*>( pOtherComponent );

            // Find children of this gameobject and change their vars if needed.
            for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
            {
                ExposedVariableDesc* pOtherVar = pOtherScript->m_ExposedVars[varindex];
                MyAssert( pOtherVar );

                if( pVar->name == pOtherVar->name )
                {
                    switch( pVar->type )
                    {
                    case ExposedVariableType::Float:
                        {
                            double oldvalue = pVar->valueDouble;
                            double newvalue = pOtherVar->valueDouble;
                            pVar->valueDouble = pOtherVar->valueDouble;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue, nullptr );

#if MYFW_USING_WX
                            m_pEngineCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue - oldvalue, PanelWatchType_Double, ((char*)&pVar->valueDouble), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
#endif
                        }
                        break;

                    case ExposedVariableType::Bool:
                        {
                            bool oldvalue = pVar->valueBool;
                            bool newvalue = pOtherVar->valueBool;
                            pVar->valueDouble = pOtherVar->valueBool;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue, nullptr );

#if MYFW_USING_WX
                            m_pEngineCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue - oldvalue, PanelWatchType_Bool, ((char*)&pVar->valueBool), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
#endif
                        }
                        break;

                    case ExposedVariableType::Vector3:
                        {
                            Vector3 oldvalue = *(Vector3*)&pVar->valueVec3;
                            Vector3 newvalue = *(Vector3*)&pOtherVar->valueVec3;
                            *(Vector3*)&pVar->valueVec3 = *(Vector3*)&pOtherVar->valueVec3;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue.x, nullptr );
                            OnExposedVarValueChanged( pVar, 1, true, oldvalue.y, nullptr );
                            OnExposedVarValueChanged( pVar, 2, true, oldvalue.z, nullptr );

#if MYFW_USING_WX
                            m_pEngineCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.x - oldvalue.x, PanelWatchType_Float, ((char*)&pVar->valueVec3[0]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
                            m_pEngineCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.y - oldvalue.y, PanelWatchType_Float, ((char*)&pVar->valueVec3[1]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ), true );
                            m_pEngineCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.z - oldvalue.z, PanelWatchType_Float, ((char*)&pVar->valueVec3[2]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ), true );
#endif
                        }
                        break;

                    case ExposedVariableType::GameObject:
                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->controlID );
                        g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pOtherVar->pointer );
#if MYFW_USING_WX
                        OnDropExposedVar( pVar->controlID, 0, 0 );
#endif
                        break;

                    case ExposedVariableType::Unused:
                    default:
                        MyAssert( false );
                        break;
                    }
                }
            }
        }
    }
}

bool ComponentScriptBase::ClearExposedVariableList(bool addUndoCommands)
{
    // Delete all variables.
    if( addUndoCommands )
    {
        // Add clear to undo stack.
        if( m_ExposedVars.size() > 0 )
        {
            m_pEngineCore->GetCommandStack()->Do(
                MyNew EditorCommand_ScriptClearExposedVariables( this, m_ExposedVars ) );

            return true;
        }
    }
    else
    {
        while( m_ExposedVars.Count() )
        {
            // Remove the first variable from the list.
            ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

            // Unregister gameobject deleted callback, if we registered one.
            if( pVariable->type == ExposedVariableType::GameObject && pVariable->pointer )
                static_cast<GameObject*>( pVariable->pointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

            delete pVariable;
        }
    }

    return false;
}
