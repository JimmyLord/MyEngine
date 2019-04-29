//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentVoxelMesh_H__
#define __ComponentVoxelMesh_H__

#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"

class VoxelChunk;

class ComponentVoxelMesh : public ComponentMesh
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentVoxelMesh );

protected:
    Vector2Int m_TextureTileCount;
    Vector3Int m_ChunkSize;
    Vector3 m_BlockSize;

public:
    ComponentVoxelMesh(ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentVoxelMesh();
    SetClassnameWith2Parents( "VoxelMeshComponent", ComponentMesh, ComponentRenderable ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    //virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentVoxelMesh&)*pObject; }
    ComponentVoxelMesh& operator=(const ComponentVoxelMesh& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual bool IsMeshReady();
    virtual void MeshFinishedLoading();

    // Rendering

    // Queries
    VoxelChunk* GetChunk() { return (VoxelChunk*)m_pMesh; }//m_pVoxelChunk; }
    void AddTileToTileInFocus(Vector2 mousepos);
    void DeleteTileInFocus(Vector2 mousepos);

protected:
    void CreateMesh();

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
#if _DEBUG
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
#endif
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
    // Runtime component variable callbacks. //_VARIABLE_LIST
    void* GetPointerValue(ComponentVariable* pVar);
    void SetPointerValue(ComponentVariable* pVar, const void* newvalue);
    const char* GetPointerDesc(ComponentVariable* pVar);
    void SetPointerDesc(ComponentVariable* pVar, const char* newdesc);

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentVoxelMesh*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);

    static void StaticOnButtonCreateMesh(void* pObjectPtr, int buttonid) { ((ComponentVoxelMesh*)pObjectPtr)->OnButtonCreateMesh( buttonid ); }
    void OnButtonCreateMesh(int buttonid);

    static void StaticOnButtonEditMesh(void* pObjectPtr, int buttonid) { ((ComponentVoxelMesh*)pObjectPtr)->OnButtonEditMesh( buttonid ); }
    void OnButtonEditMesh(int buttonid);
#endif //MYFW_EDITOR
};

#endif //__ComponentVoxelMesh_H__
