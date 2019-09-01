//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentScriptBase_H__
#define __ComponentScriptBase_H__

#include "ComponentSystem/BaseComponents/ComponentUpdateable.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"

enum class ExposedVariableType
{
    Unused,
    Float,
    Bool,
    Vector3,
    GameObject,
};

class ExposedVariableValue
{
public:
    ExposedVariableType type;
    union // TODO?: Make these values shared between c++ and the script so they can be changed/saved more easily.
    {
        double valueDouble;
        bool valueBool;
        Vector3 valueVec3;
        void* valuePointer;
    };

    ExposedVariableValue()
    {
        Reset();
    }

    ExposedVariableValue(float value)
    {
        Reset();
        type = ExposedVariableType::Float;
        valueDouble = value;
    }

    ExposedVariableValue(bool value)
    {
        Reset();
        type = ExposedVariableType::Bool;
        valueBool = value;
    }

    void Reset()
    {
        type = ExposedVariableType::Unused;
        valueDouble = 0;
        valueBool = 0;
        valueVec3.Set( 0, 0, 0 );
        valuePointer = nullptr;
    }
};

class ExposedVariableDesc
{
public:
    std::string name;
    ExposedVariableValue value;
    bool divorced;
    bool inUse; // Used internally when reparsing the file.
    int controlID;

    ExposedVariableDesc()
    {
        Reset();
    }

    void Reset()
    {
        name = "";
        value.Reset();
        divorced = false;
        inUse = false;
        controlID = -1;
    }
};

typedef void ExposedVarValueChangedCallback(void* pObjectPtr, ExposedVariableDesc* pVar, int component, bool finishedChanging, ExposedVariableValue oldValue, void* oldPointer);

class ComponentScriptBase : public ComponentUpdateable
{
private:
    static const int MAX_EXPOSED_VARS = 4; // TODO: Fix this hardcodedness.

protected:
    MyList<ExposedVariableDesc*> m_ExposedVars;

public:
    ComponentScriptBase(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentScriptBase();

    cJSON* ExportExposedVariablesAsJSONObject();
    void ImportExposedVariablesFromJSONObject(cJSON* jExposedVarArray);

    void AddExposedVariablesToInterface();

    void CopyExposedVariablesFromOtherComponent(const ComponentScriptBase& other);

    virtual void ProgramVariables(bool updateExposedVariables = false) = 0;

    // GameObject callbacks.
    static void StaticOnGameObjectDeleted(void* pObjectPtr, GameObject* pGameObject) { ((ComponentScriptBase*)pObjectPtr)->OnGameObjectDeleted( pGameObject ); }
    virtual void OnGameObjectDeleted(GameObject* pGameObject) = 0;

    // Exposed variable changed callback.
    static void StaticOnExposedVarValueChanged(void* pObjectPtr, ExposedVariableDesc* pVar, int component, bool finishedChanging, ExposedVariableValue oldValue, void* oldPointer) { ((ComponentScriptBase*)pObjectPtr)->OnExposedVarValueChanged( pVar, component, finishedChanging, oldValue, oldPointer ); }
    virtual void OnExposedVarValueChanged(ExposedVariableDesc* pVar, int component, bool finishedChanging, ExposedVariableValue oldValue, void* oldPointer) = 0;

#if MYFW_EDITOR
protected:
#if MYFW_USING_IMGUI
    // For TestForVariableModificationAndCreateUndoCommand.
    double m_ValueWhenControlSelected;
    ImGuiID m_ImGuiControlIDForCurrentlySelectedVariable;
    bool m_LinkNextUndoCommandToPrevious;

    static void TestForExposedVariableModificationAndCreateUndoCommand(ComponentScriptBase* pComponent, ImGuiID id, bool modified, ExposedVariableDesc* pVar, ExposedVariableValue newValue);
#endif //MYFW_USING_IMGUI

    bool DoesExposedVariableMatchParent(ExposedVariableDesc* pVar);
    void UpdateChildrenWithNewValue(ExposedVariableDesc* pVar, bool finishedChanging, double oldValue, void* oldPointer);
    void UpdateChildrenInGameObjectListWithNewValue(ExposedVariableDesc* pVar, unsigned int varindex, GameObject* first, bool finishedChanging, double oldValue, void* oldPointer);
    void UpdateChildGameObjectWithNewValue(ExposedVariableDesc* pVar, unsigned int varIndex, GameObject* pChildGameObject, bool finishedChanging, double oldValue, void* oldPointer);
    void CopyExposedVarValueFromParent(ExposedVariableDesc* pVar);

    bool ClearExposedVariableList(bool addUndoCommands);
#endif //MYFW_EDITOR
};

#endif //__ComponentScriptBase_H__
