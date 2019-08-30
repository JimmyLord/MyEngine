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

class ExposedVariableDesc
{
public:
    std::string name;
    ExposedVariableType type;
    union // TODO?: Make these values shared between c++ and the script so they can be changed/saved more easily.
    {
        double valueDouble;
        bool valueBool;
        Vector3 valueVec3;
        void* pointer;
    };

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
        type = ExposedVariableType::Unused;
        valueDouble = 0;
        valueBool = 0;
        valueVec3.Set( 0, 0, 0 );
        divorced = false;
        inUse = false;
        controlID = -1;
    }
};

typedef void ExposedVarValueChangedCallback(void* pObjectPtr, ExposedVariableDesc* pVar, int component, bool finishedChanging, double oldValue, void* oldPointer);

class ComponentScriptBase : public ComponentUpdateable
{
private:
    static const int MAX_EXPOSED_VARS = 4; // TODO: Fix this hardcodedness.

protected:
    MyList<ExposedVariableDesc*> m_ExposedVars;

public:
    ComponentScriptBase(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentScriptBase();

    // GameObject callbacks.
    static void StaticOnGameObjectDeleted(void* pObjectPtr, GameObject* pGameObject) { ((ComponentScriptBase*)pObjectPtr)->OnGameObjectDeleted( pGameObject ); }
    virtual void OnGameObjectDeleted(GameObject* pGameObject) = 0;
};

#endif //__ComponentScriptBase_H__
