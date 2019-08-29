//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineEditorCommands_H__
#define __EngineEditorCommands_H__

#include "ComponentSystem/EngineComponents/ComponentScriptBase.h"
#include "ComponentSystem/FrameworkComponents/ComponentLuaScript.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshPrimitive.h"

class Component2DCollisionObject;
class ComponentAudioPlayer;
class ComponentRenderable;
class ExposedVariableDesc;

class EditorCommand_ImGuiPanelWatchNumberValueChanged;
class EditorCommand_ImGuiPanelWatchColorChanged;
class EditorCommand_ImGuiPanelWatchPointerChanged;
class EditorCommand_MoveObjects;
class EditorCommand_ScaleObjects;
class EditorCommand_RotateObjects;
class EditorCommand_DeleteObjects;
class EditorCommand_DeleteComponents;
class EditorCommand_CreateGameObject;
class EditorCommand_CreateComponent;
class EditorCommand_CopyGameObject;
class EditorCommand_ClearParentOfGameObjects;
class EditorCommand_EnableObject;
class EditorCommand_DragAndDropEvent;
class EditorCommand_ChangeMaterialOnMesh;
class EditorCommand_ChangeTextureOnMaterial;
class EditorCommand_ChangeShaderOnMaterial;
class EditorCommand_ChangeAllScriptsOnGameObject;
class EditorCommand_ChangeSoundCue;
class EditorCommand_Move2DPoint;
class EditorCommand_Insert2DPoint;
class EditorCommand_Delete2DPoint;
class EditorCommand_ComponentVariablePointerChanged;
class EditorCommand_LuaExposedVariableFloatChanged;
class EditorCommand_LuaExposedVariablePointerChanged;
class EditorCommand_LuaClearExposedVariables;
class EditorCommand_DeletePrefabs;
class EditorCommand_DivorceOrMarryComponentVariable;
class EditorCommand_ComponentVariableIndirectPointerChanged;
class EditorCommand_ReorderOrReparentGameObjects;
class EditorCommand_RestorePrefabComponent;
class EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh;

#if MYFW_USING_IMGUI
typedef void PanelWatchCallbackValueChanged(void* pObjectPtr, int controlID, bool directlyChanged, bool finishedChanging, double oldValue, bool valueWasChangedByDragging);

enum PanelWatch_Types
{
    PanelWatchType_Int,
    PanelWatchType_UnsignedInt,
    PanelWatchType_Char,
    PanelWatchType_UnsignedChar,
    PanelWatchType_Bool,
    PanelWatchType_Float,
    PanelWatchType_Double,
    //PanelWatchType_Vector3,
    PanelWatchType_ColorFloat,
    PanelWatchType_ColorByte,
    PanelWatchType_PointerWithDesc,
    PanelWatchType_Enum,
    PanelWatchType_Flags,
    PanelWatchType_SpaceWithLabel,
    PanelWatchType_Button,
    PanelWatchType_String,

    //ADDING_NEW_WatchVariableType

    PanelWatchType_Unknown,
    PanelWatchType_NumTypes,
};

class EditorCommand_ImGuiPanelWatchNumberValueChanged : public EditorCommand
{
protected:
    void* m_pObject;
    ComponentBase* m_pComponent;
    ComponentVariable* m_pVar;

    ComponentVariableValue m_NewValue;
    ComponentVariableValue m_OldValue;
    bool m_DirectlyChanged;

public:
    EditorCommand_ImGuiPanelWatchNumberValueChanged(void* pObject, ComponentVariable* pVar, ComponentVariableValue newValue, ComponentVariableValue oldValue, bool directlyChanged, ComponentBase* pComponent);
    virtual ~EditorCommand_ImGuiPanelWatchNumberValueChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();

    bool UsesThisObjectAndVariable(void* pObject, ComponentVariable* pVar) { return m_pObject == pObject && m_pVar == pVar; }
};

//====================================================================================================

class EditorCommand_ImGuiPanelWatchColorChanged : public EditorCommand
{
protected:
    ColorFloat m_NewColor;
    ColorFloat m_OldColor;
    PanelWatch_Types m_Type;
    void* m_Pointer;
    int m_ControlID;
    bool m_DirectlyChanged;

    PanelWatchCallbackValueChanged* m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_ImGuiPanelWatchColorChanged(ColorFloat newColor, PanelWatch_Types type, void* pPointer, int controlID, bool directlyChanged, PanelWatchCallbackValueChanged* callbackFunc, void* callbackObj);
    virtual ~EditorCommand_ImGuiPanelWatchColorChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ImGuiPanelWatchPointerChanged : public EditorCommand
{
protected:
    void* m_NewValue;
    void* m_OldValue;
    PanelWatch_Types m_Type;
    void** m_pPointer;
    int m_ControlID;
    bool m_DirectlyChanged;

    PanelWatchCallbackValueChanged* m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_ImGuiPanelWatchPointerChanged(void* newValue, PanelWatch_Types type, void** pPointer, int controlID, bool directlyChanged, PanelWatchCallbackValueChanged* callbackFunc, void* callbackObj);
    virtual ~EditorCommand_ImGuiPanelWatchPointerChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};
#endif //MYFW_USING_IMGUI

//====================================================================================================

class EditorCommand_MoveObjects : public EditorCommand
{
protected:
    Vector3 m_DistanceMoved;
    std::vector<GameObject*> m_ObjectsMoved;

public:
    EditorCommand_MoveObjects(Vector3 distancemoved, const std::vector<GameObject*>& selectedobjects);
    virtual ~EditorCommand_MoveObjects();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ScaleObjects : public EditorCommand
{
protected:
    Vector3 m_AmountScaled;
    bool m_TransformedInLocalSpace;
    Vector3 m_WorldSpacePivot;
    std::vector<GameObject*> m_ObjectsScaled;

public:
    EditorCommand_ScaleObjects(Vector3 amountscaled, bool localspace, Vector3 pivot, const std::vector<GameObject*>& selectedobjects);
    virtual ~EditorCommand_ScaleObjects();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_RotateObjects : public EditorCommand
{
protected:
    Vector3 m_AmountRotated;
    bool m_TransformedInLocalSpace;
    Vector3 m_WorldSpacePivot;
    std::vector<GameObject*> m_ObjectsRotated;

public:
    EditorCommand_RotateObjects(Vector3 amountRotated, bool localspace, Vector3 pivot, const std::vector<GameObject*>& selectedobjects);
    virtual ~EditorCommand_RotateObjects();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_DeleteObjects : public EditorCommand
{
protected:
    // TODO: likely change this whole command to store the full editor state and restore on undo.
    //       otherwise all references to this object will need to be restored

    // IF this is in undo stack, then this stores the only reference to the deleted object.
    //                                and the gameobject stores the only references to the components it contained.
    std::vector<GameObject*> m_PreviousGameObjectsInObjectList; // to reinsert at the right place.
    std::vector<GameObject*> m_ObjectsDeleted;
    bool m_DeleteGameObjectsWhenDestroyed;

public:
    EditorCommand_DeleteObjects(const std::vector<GameObject*>& selectedobjects);
    virtual ~EditorCommand_DeleteObjects();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_DeleteComponents : public EditorCommand
{
protected:
    // TODO: likely change this whole command to store the full editor state and restore on undo.
    //       otherwise all references to this object will need to be restored

    // IF this is in undo stack, then this stores the only reference to the deleted object.
    //                                and the gameobject stores the only references to the components it contained.
    std::vector<ComponentBase*> m_ComponentsDeleted;
    std::vector<bool> m_ComponentWasDisabled;
    bool m_DeleteComponentsWhenDestroyed;

public:
    EditorCommand_DeleteComponents(const std::vector<ComponentBase*>& selectedComponents);
    virtual ~EditorCommand_DeleteComponents();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_CreateGameObject : public EditorCommand
{
protected:
    GameObject* m_ObjectCreated;
    bool m_DeleteGameObjectsWhenDestroyed;

public:
    EditorCommand_CreateGameObject(GameObject* objectcreated);
    virtual ~EditorCommand_CreateGameObject();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();

    GameObject* GetCreatedObject() { return m_ObjectCreated; }
};

//====================================================================================================

class EditorCommand_CreateComponent : public EditorCommand
{
protected:
    GameObject* m_pGameObject;
    int m_ComponentType;

    ComponentBase* m_pComponentCreated;
    bool m_DeleteComponentWhenDestroyed;

public:
    EditorCommand_CreateComponent(GameObject* pGameObject, int componentType);
    virtual ~EditorCommand_CreateComponent();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();

    ComponentBase* GetCreatedObject() { return m_pComponentCreated; }
};

//====================================================================================================

class EditorCommand_CopyGameObject : public EditorCommand
{
protected:
    // IF this is in redo stack, then m_ObjectCreated stores the only reference to the gameobject.
    //                                and the gameobject stores the only references to the components it contained.

    // TODO: make this an array, for cases where multiple objects are copied in one action.
    GameObject* m_ObjectToCopy; // solely for the Repeat() function to repeat.
    GameObject* m_ObjectCreated;
    bool m_NewObjectInheritsFromOld;
    bool m_DeleteGameObjectWhenDestroyed;

public:
    EditorCommand_CopyGameObject(GameObject* objecttocopy, bool NewObjectInheritsFromOld);
    virtual ~EditorCommand_CopyGameObject();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();

    GameObject* GetCreatedObject() { return m_ObjectCreated; }
};

//====================================================================================================

class EditorCommand_ClearParentOfGameObjects : public EditorCommand
{
protected:
    std::vector<GameObject*> m_pObjectsToClear;
    std::vector<GameObject*> m_pOldParents;
    std::vector<PrefabReference> m_OldPrefabRefs;

public:
    EditorCommand_ClearParentOfGameObjects(GameObject* pObjectToClear);
    EditorCommand_ClearParentOfGameObjects(std::vector<GameObject*>* pObjectsToClear);
    virtual ~EditorCommand_ClearParentOfGameObjects();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_EnableObject : public EditorCommand
{
protected:
    GameObject* m_pGameObject;
    bool m_ObjectWasEnabled;
    bool m_AffectChildren;

public:
    EditorCommand_EnableObject(GameObject* pObject, bool enabled, bool affectChildren);
    virtual ~EditorCommand_EnableObject();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_DragAndDropEvent : public EditorCommand
{
protected:
    ComponentBase* m_pComponent;
    ComponentVariable* m_pVar;
    int m_ControlComponent;
    int m_X;
    int m_Y;

    DragAndDropTypes m_Type;
    void* m_pNewValue;
    void* m_pOldValue;

public:
    EditorCommand_DragAndDropEvent(ComponentBase* pComponent, ComponentVariable* pVar, int controlcomponent, int x, int y, DragAndDropTypes type, void* newValue, void* oldValue);
    virtual ~EditorCommand_DragAndDropEvent();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ChangeMaterialOnMesh : public EditorCommand
{
protected:
    ComponentRenderable* m_pComponent;
    int m_SubmeshIndex;
    MaterialDefinition* m_pNewMaterial;
    MaterialDefinition* m_pOldMaterial;

    ComponentVariable* m_pVar;
    bool m_VariableWasDivorced;

public:
    EditorCommand_ChangeMaterialOnMesh(ComponentRenderable* pComponent, ComponentVariable* pVar, int submeshindex, MaterialDefinition* pMaterial);
    virtual ~EditorCommand_ChangeMaterialOnMesh();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ChangeTextureOnMaterial : public EditorCommand
{
protected:
    MaterialDefinition* m_pMaterial;
    TextureDefinition* m_pNewTexture;

    TextureDefinition* m_pOldTexture;

public:
    EditorCommand_ChangeTextureOnMaterial(MaterialDefinition* material, TextureDefinition* texture);
    virtual ~EditorCommand_ChangeTextureOnMaterial();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ChangeShaderOnMaterial : public EditorCommand
{
protected:
    MaterialDefinition* m_pMaterial;
    ShaderGroup* m_pNewShaderGroup;

    ShaderGroup* m_pOldShaderGroup;

public:
    EditorCommand_ChangeShaderOnMaterial(MaterialDefinition* material, ShaderGroup* shadergroup);
    virtual ~EditorCommand_ChangeShaderOnMaterial();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ChangeAllScriptsOnGameObject : public EditorCommand
{
protected:
    GameObject* m_pGameObject;
    MyFileObject* m_pNewScriptFile;

    std::vector<ComponentBase*> m_ComponentsChanged;
    std::vector<MyFileObject*> m_OldScriptFiles;

public:
    EditorCommand_ChangeAllScriptsOnGameObject(GameObject* object, MyFileObject* scriptfile);
    virtual ~EditorCommand_ChangeAllScriptsOnGameObject();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ChangeSoundCue : public EditorCommand
{
protected:
    ComponentAudioPlayer* m_pComponent;
    SoundCue* m_pSoundCue;
    SoundCue* m_pOldSoundCue;

public:
    EditorCommand_ChangeSoundCue(ComponentAudioPlayer* pComponent, SoundCue* pSoundCue);
    virtual ~EditorCommand_ChangeSoundCue();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_Move2DPoint : public EditorCommand
{
protected:
    Component2DCollisionObject* m_pCollisionObject;
    b2Vec2 m_DistanceMoved;
    int m_IndexOfPointMoved;

public:
    EditorCommand_Move2DPoint(b2Vec2 distancemoved, Component2DCollisionObject* pCollisionObject, int indexmoved);
    virtual ~EditorCommand_Move2DPoint();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_Insert2DPoint : public EditorCommand
{
protected:
    Component2DCollisionObject* m_pCollisionObject;
    int m_IndexOfPointInserted;

public:
    EditorCommand_Insert2DPoint(Component2DCollisionObject* pCollisionObject, int indexinserted);
    virtual ~EditorCommand_Insert2DPoint();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_Delete2DPoint : public EditorCommand
{
protected:
    Component2DCollisionObject* m_pCollisionObject;
    int m_IndexOfPointDeleted;
    b2Vec2 m_Position;

public:
    EditorCommand_Delete2DPoint(Component2DCollisionObject* pCollisionObject, int indexdeleted, b2Vec2 position);
    virtual ~EditorCommand_Delete2DPoint();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ComponentVariablePointerChanged : public EditorCommand
{
protected:
    ComponentBase* m_pComponent;
    ComponentVariable* m_pVar;

    ComponentVariableValue m_NewPointer;
    ComponentVariableValue m_OldPointer;

public:
    EditorCommand_ComponentVariablePointerChanged(ComponentBase* pComponent, ComponentVariable* pVar, ComponentVariableValue* pNewValue);
    virtual ~EditorCommand_ComponentVariablePointerChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ScriptExposedVariableFloatChanged : public EditorCommand
{
protected:
    double m_NewValue;
    double m_OldValue;
    ScriptExposedVariableDesc* m_pVar;

    ScriptExposedVarValueChangedCallback* m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_ScriptExposedVariableFloatChanged(double newValue, ScriptExposedVariableDesc* pVar, ScriptExposedVarValueChangedCallback* callbackFunc, void* callbackObj);
    virtual ~EditorCommand_ScriptExposedVariableFloatChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_LuaExposedVariableFloatChanged : public EditorCommand
{
protected:
    double m_NewValue;
    double m_OldValue;
    ExposedVariableDesc* m_pVar;

    LuaExposedVarValueChangedCallback* m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_LuaExposedVariableFloatChanged(double newValue, ExposedVariableDesc* pVar, LuaExposedVarValueChangedCallback* callbackFunc, void* callbackObj);
    virtual ~EditorCommand_LuaExposedVariableFloatChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_LuaExposedVariablePointerChanged : public EditorCommand
{
protected:
    void* m_NewValue;
    void* m_OldValue;
    ExposedVariableDesc* m_pVar;

    LuaExposedVarValueChangedCallback* m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_LuaExposedVariablePointerChanged(void* newValue, ExposedVariableDesc* pVar, LuaExposedVarValueChangedCallback* callbackFunc, void* callbackObj);
    virtual ~EditorCommand_LuaExposedVariablePointerChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_LuaClearExposedVariables : public EditorCommand
{
protected:
    ComponentLuaScript* m_pLuaScriptComponent;
    MyList<ExposedVariableDesc*>& m_OriginalExposedVariablesListFromComponent;
    std::vector<ExposedVariableDesc*> m_CopyOfExposedVariables;
    bool m_DeleteExposedVarsWhenDestroyed;

public:
    EditorCommand_LuaClearExposedVariables(ComponentLuaScript* pLuaScriptComponent, MyList<ExposedVariableDesc*>& exposedVariablesList);
    virtual ~EditorCommand_LuaClearExposedVariables();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_DeletePrefabs : public EditorCommand
{
protected:
    struct PrefabInfo
    {
        PrefabObject* m_pPrefab;
        PrefabObject* m_pPreviousPrefabInObjectList; // to reinsert at the right place.
        std::vector<GameObject*> m_pListOfGameObjectsThatUsedPrefab; // for undo
        std::vector<PrefabReference> m_CopyOfPrefabRefsInEachGameObjectBeforePrefabWasDeleted;
    };

    // IF this is in undo stack, then this stores the only reference to the deleted prefab.
    std::vector<PrefabInfo> m_PrefabInfo;
    bool m_DeletePrefabsWhenDestroyed;

public:
    EditorCommand_DeletePrefabs(const std::vector<PrefabObject*>& selectedprefabs);
    virtual ~EditorCommand_DeletePrefabs();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_DivorceOrMarryComponentVariable : public EditorCommand
{
protected:
    ComponentBase* m_pComponent;
    ComponentVariable* m_pVar;
    ComponentVariableValue m_OldValue;
    ComponentVariableValue m_NewValue;

    bool m_DivorceTheVariable;

public:
    EditorCommand_DivorceOrMarryComponentVariable(ComponentBase* pComponent, ComponentVariable* pVar, bool divorcethevariable);
    virtual ~EditorCommand_DivorceOrMarryComponentVariable();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ComponentVariableIndirectPointerChanged : public EditorCommand
{
protected:
    ComponentBase* m_pComponent;
    ComponentVariable* m_pVar;
    void* m_OldValue;
    void* m_NewValue;

public:
    EditorCommand_ComponentVariableIndirectPointerChanged(ComponentBase* pComponent, ComponentVariable* pVar, void* newvalue);
    virtual ~EditorCommand_ComponentVariableIndirectPointerChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ReorderOrReparentGameObjects : public EditorCommand
{
protected:
    std::vector<GameObject*> m_SelectedObjects;
    GameObject* m_pObjectDroppedOn;
    SceneID m_SceneIDDroppedOn;
    bool m_MakeSelectedObjectsChildren;

    std::vector<SceneID> m_OldSceneIDs;
    std::vector<GameObject*> m_OldPreviousObjectInList;
    std::vector<GameObject*> m_OldParent;

public:
    EditorCommand_ReorderOrReparentGameObjects(const std::vector<GameObject*>& selectedobjects, GameObject* pObjectDroppedOn, SceneID sceneid, bool setaschild);
    virtual ~EditorCommand_ReorderOrReparentGameObjects();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_RestorePrefabComponent : public EditorCommand
{
protected:
    GameObject* m_pGameObject;
    uint32 m_DeletedPrefabComponentID;
    ComponentBase* m_pComponentCreated;

public:
    EditorCommand_RestorePrefabComponent(GameObject* pObject, uint32 deletedPrefabComponentID);
    virtual ~EditorCommand_RestorePrefabComponent();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh : public EditorCommand
{
protected:
    ComponentMeshPrimitive* m_pComponent;
    MyMesh* m_pOldMesh;
    MyMesh* m_pNewMesh;
    MyRE::PrimitiveTypes m_OldGLPrimitiveType;
    MyRE::PrimitiveTypes m_NewGLPrimitiveType;
    ComponentMeshPrimitives m_NewMeshPrimitiveType;

public:
    EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh(ComponentMeshPrimitive* pComponent, MyMesh* pOldMesh, ComponentMeshPrimitives newMeshPrimitiveType);
    virtual ~EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

#endif // __EngineEditorCommands_H__
