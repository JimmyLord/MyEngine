//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineEditorCommands_H__
#define __EngineEditorCommands_H__

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
    bool m_DeleteComponentsWhenDestroyed;

public:
    EditorCommand_DeleteComponents(const std::vector<ComponentBase*>& selectedcomponents);
    virtual ~EditorCommand_DeleteComponents();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
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

class EditorCommand_EnableObject : public EditorCommand
{
protected:
    GameObject* m_pGameObject;
    bool m_ObjectWasEnabled;

public:
    EditorCommand_EnableObject(GameObject* pObject, bool enabled);
    virtual ~EditorCommand_EnableObject();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_ChangeAllMaterialsOnGameObject : public EditorCommand
{
protected:
    GameObject* m_pGameObject;
    MaterialDefinition* m_pNewMaterial;

    std::vector<ComponentBase*> m_ComponentsChanged;
    std::vector<MaterialDefinition*> m_OldMaterials;

public:
    EditorCommand_ChangeAllMaterialsOnGameObject(GameObject* object, MaterialDefinition* material);
    virtual ~EditorCommand_ChangeAllMaterialsOnGameObject();

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
    MyFileObject* pNewScriptFile;

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

#endif // __EngineEditorCommands_H__
