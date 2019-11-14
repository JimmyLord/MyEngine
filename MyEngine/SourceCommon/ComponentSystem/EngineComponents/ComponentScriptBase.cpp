//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentScriptBase.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "GUI/ImGuiExtensions.h"

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
        cJSON_AddNumberToObject( jExposedVar, "Type", (int)pVar->value.type );

        if( pVar->value.type == ExposedVariableType::Float )
        {
            cJSON_AddNumberToObject( jExposedVar, "Value", pVar->value.valueDouble );
        }
        if( pVar->value.type == ExposedVariableType::Bool )
        {
            cJSON_AddNumberToObject( jExposedVar, "Value", pVar->value.valueBool );
        }
        if( pVar->value.type == ExposedVariableType::Vector3 )
        {
            cJSONExt_AddFloatArrayToObject( jExposedVar, "Value", &pVar->value.valueVec3.x, 3 );
        }
        else if( pVar->value.type == ExposedVariableType::GameObject && pVar->value.valuePointer )
        {
            cJSON* jGameObjectRef = static_cast<GameObject*>( pVar->value.valuePointer )->ExportReferenceAsJSONObject( m_SceneIDLoadedFrom );
            cJSON_AddItemToObject( jExposedVar, "Value", jGameObjectRef );
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
        cJSONExt_GetInt( jsonvar, "Type", (int*)&pVar->value.type );

        if( pVar->value.type == ExposedVariableType::Float )
        {
            cJSONExt_GetDouble( jsonvar, "Value", &pVar->value.valueDouble );
        }
        if( pVar->value.type == ExposedVariableType::Bool )
        {
            cJSONExt_GetBool( jsonvar, "Value", &pVar->value.valueBool );
        }
        if( pVar->value.type == ExposedVariableType::Vector3 )
        {
            cJSONExt_GetFloatArray( jsonvar, "Value", &pVar->value.valueVec3.x, 3 );
        }
        else if( pVar->value.type == ExposedVariableType::GameObject )
        {
            cJSON* obj = cJSON_GetObjectItem( jsonvar, "Value" );
            if( obj )
            {
                pVar->value.valuePointer = m_pComponentSystemManager->FindGameObjectByJSONRef( obj, m_pGameObject->GetSceneID(), false );

                // TODO: Handle cases where the Scene containing the GameObject referenced isn't loaded.
                //MyAssert( pVar->pointer != nullptr );
                if( pVar->value.valuePointer )
                {
                    static_cast<GameObject*>( pVar->value.valuePointer )->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
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
            if( pVar->value.type == ExposedVariableType::Float )
            {
                pVar->value.valueDouble = pOtherVar->value.valueDouble;
            }

            if( pVar->value.type == ExposedVariableType::Bool )
            {
                pVar->value.valueBool = pOtherVar->value.valueBool;
            }

            if( pVar->value.type == ExposedVariableType::Vector3 )
            {
                for( int i=0; i<3; i++ )
                    pVar->value.valueVec3[0] = pOtherVar->value.valueVec3[0];
            }

            if( pVar->value.type == ExposedVariableType::GameObject )
            {
                pVar->value.valuePointer = pOtherVar->value.valuePointer;

                if( pVar->value.valuePointer )
                    static_cast<GameObject*>( pVar->value.valuePointer )->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
            }
        }
    }
}

#if MYFW_EDITOR
void ComponentScriptBase::AddExposedVariablesToInterface()
{
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        switch( pVar->value.type )
        {
        case ExposedVariableType::Unused:
            MyAssert( false );
            break;

        case ExposedVariableType::Float:
            {
                float tempValue = (float)pVar->value.valueDouble;
                bool modified = ImGui::DragFloat( pVar->name.c_str(), &tempValue, 0.1f );
                if( modified )
                {
                    TestForExposedVariableModificationAndCreateUndoCommand( this, ImGuiExt::GetActiveItemId(), modified, pVar, ExposedVariableValue(tempValue) );
                }
            }
            break;

        case ExposedVariableType::Bool:
            {
                bool tempValue = pVar->value.valueBool;
                bool modified = ImGui::Checkbox( pVar->name.c_str(), &tempValue );
                if( modified )
                {
                    TestForExposedVariableModificationAndCreateUndoCommand( this, ImGuiExt::GetLastItemId(), modified, pVar, ExposedVariableValue(tempValue) );
                }
            }
            break;

        case ExposedVariableType::Vector3:
            {
                Vector3 tempValue = pVar->value.valueVec3;
                bool modified = ImGui::DragFloat3( pVar->name.c_str(), &tempValue.x );
                if( modified )
                {
                    TestForExposedVariableModificationAndCreateUndoCommand( this, ImGuiExt::GetLastItemId(), modified, pVar, ExposedVariableValue(tempValue) );
                }
            }
            break;

        case ExposedVariableType::GameObject:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                GameObject* pGameObject = static_cast<GameObject*>( pVar->value.valuePointer );

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

// Static.
void ComponentScriptBase::TestForExposedVariableModificationAndCreateUndoCommand(ComponentScriptBase* pComponent, ImGuiID id, bool modified, ExposedVariableDesc* pVar, ExposedVariableValue newValue)
{
    MyAssert( pComponent != nullptr );

    // If the id passed in is different than the last known value, then assume a new control was selected.
    if( id != pComponent->m_ImGuiControlIDForCurrentlySelectedVariable )
    {
        // If a new control was selected, store the starting value and start a new undo chain.
        pComponent->m_ValueWhenControlSelected = pVar->value.valueDouble;
        pComponent->m_LinkNextUndoCommandToPrevious = false;
        pComponent->m_ImGuiControlIDForCurrentlySelectedVariable = id;
    }

    // If the control returned true to indicate it was modified, then create an undo command.
    if( modified && id != 0 )
    {
        MyAssert( id == pComponent->m_ImGuiControlIDForCurrentlySelectedVariable );

        // Add an undo action.
        EditorCommand* pCommand = MyNew EditorCommand_ExposedVariableChanged(
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
                    switch( pVar->value.type )
                    {
                    case ExposedVariableType::Float:
                        return pVar->value.valueDouble == pOtherVar->value.valueDouble;

                    case ExposedVariableType::Bool:
                        return pVar->value.valueBool == pOtherVar->value.valueBool;

                    case ExposedVariableType::Vector3:
                        return pVar->value.valueVec3 == pOtherVar->value.valueVec3;

                    case ExposedVariableType::GameObject:
                        return pVar->value.valuePointer == pOtherVar->value.valuePointer;

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
                    if( pChildVar->value.type == ExposedVariableType::Float ||
                        pChildVar->value.type == ExposedVariableType::Bool )
                    {
                        if( fequal( pChildVar->value.valueDouble, oldValue ) )
                        {
                            pChildVar->value.valueDouble = pVar->value.valueDouble;
                            //pChildScript->OnExposedVarValueChanged( controlid, finishedchanging, oldvalue );

                            pChildScript->ProgramVariables( true );
                            pChildScript->UpdateChildrenWithNewValue( pChildVar, finishedChanging, oldValue, oldPointer );
                        }
                    }

                    if( pChildVar->value.type == ExposedVariableType::Vector3 )
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

                    if( pVar->value.type == ExposedVariableType::GameObject )
                    {
                        if( pChildVar->value.valuePointer == oldPointer )
                        {
                            pChildVar->value.valuePointer = pVar->value.valuePointer;
                            if( pVar->value.valuePointer )
                                static_cast<GameObject*>( pVar->value.valuePointer )->RegisterOnDeleteCallback( pChildScript, StaticOnGameObjectDeleted );

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
                    switch( pVar->value.type )
                    {
                    case ExposedVariableType::Float:
                        {
                            double oldvalue = pVar->value.valueDouble;
                            double newvalue = pOtherVar->value.valueDouble;
                            pVar->value.valueDouble = pOtherVar->value.valueDouble;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, ExposedVariableValue((float)oldvalue), nullptr );
                        }
                        break;

                    case ExposedVariableType::Bool:
                        {
                            bool oldvalue = pVar->value.valueBool;
                            bool newvalue = pOtherVar->value.valueBool;
                            pVar->value.valueDouble = pOtherVar->value.valueBool;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, ExposedVariableValue(oldvalue), nullptr );
                        }
                        break;

                    case ExposedVariableType::Vector3:
                        {
                            Vector3 oldvalue = *(Vector3*)&pVar->value.valueVec3;
                            Vector3 newvalue = *(Vector3*)&pOtherVar->value.valueVec3;
                            *(Vector3*)&pVar->value.valueVec3 = *(Vector3*)&pOtherVar->value.valueVec3;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue.x, nullptr );
                            OnExposedVarValueChanged( pVar, 1, true, oldvalue.y, nullptr );
                            OnExposedVarValueChanged( pVar, 2, true, oldvalue.z, nullptr );
                        }
                        break;

                    case ExposedVariableType::GameObject:
                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->controlID );
                        g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pOtherVar->value.valuePointer );
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
            if( pVariable->value.type == ExposedVariableType::GameObject && pVariable->value.valuePointer )
                static_cast<GameObject*>( pVariable->value.valuePointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

            delete pVariable;
        }
    }

    return false;
}
#endif //MYFW_EDITOR
