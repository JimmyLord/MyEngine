//
// Copyright (c) 2019-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentHeightmap_H__
#define __ComponentHeightmap_H__

#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"

class ComponentHeightmap : public ComponentMesh
{
    friend class EditorDocument_Heightmap;
    friend class EditorCommand_Heightmap_Raise;
    friend class EditorCommand_Heightmap_FullBackup;
    friend class Job_CalculateNormals;

private:
    // Component Variable List.
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentHeightmap );

protected:
    Vector2 m_Size;
    Vector2Int m_VertCount;
    float* m_Heights;

    MyFileObject* m_pHeightmapFile;
    Vector2Int m_HeightmapFileSize;
    bool m_WaitingForHeightmapFileToFinishLoading;

    TextureDefinition* m_pHeightmapTexture;
    Vector2Int m_HeightmapTextureSize;
    bool m_WaitingForTextureFileToFinishLoading;

public:
    ComponentHeightmap(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentHeightmap();
    SetClassnameWith2Parents( "HeightmapComponent", ComponentMesh, ComponentRenderable );

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luaState);
#endif //MYFW_USING_LUA

    //virtual cJSON* ExportAsJSONObject(bool saveSceneID, bool saveID);
    //virtual void ImportFromJSONObject(cJSON* jComponent, SceneID sceneID);
    virtual void FinishImportingFromJSONObject(cJSON* jComponent) override;

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentHeightmap&)*pObject; }
    ComponentHeightmap& operator=(const ComponentHeightmap& other);

    virtual void OnLoad();

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

protected:
#if MYFW_EDITOR
    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);

    void OnButtonEditHeightmap();
#endif //MYFW_EDITOR

    // Getters.
    MyFileObject* GetHeightmapFile() { return m_pHeightmapFile; }

    // Setters.
    void SetHeightmapFile(MyFileObject* pFile);
    void UnregisterHeightmapFileLoadingCallbacks(bool force);
    static void StaticOnFileFinishedLoadingHeightmapFile(void* pObjectPtr, MyFileObject* pFile) { ((ComponentHeightmap*)pObjectPtr)->OnFileFinishedLoadingHeightmapFile( pFile ); }
    void OnFileFinishedLoadingHeightmapFile(MyFileObject* pFile);

    void SetHeightmapTexture(TextureDefinition* pTexture);
    void UnregisterHeightmapTextureLoadingCallbacks(bool force);
    static void StaticOnFileFinishedLoadingHeightmapTexture(void* pObjectPtr, MyFileObject* pFile) { ((ComponentHeightmap*)pObjectPtr)->OnFileFinishedLoadingHeightmapTexture( pFile ); }
    void OnFileFinishedLoadingHeightmapTexture(MyFileObject* pFile);

    // 
    void CreateHeightmap();
    bool GenerateHeightmapMeshFromTexture(bool sizeChanged, bool rebuildNormals);
    bool GenerateHeightmapMesh(bool sizeChanged, bool rebuildNormals);
    void FillWithNoise(int noiseSeed, float amplitude, Vector2 frequency, Vector2 offset, int octaves, float persistance, float lacunarity);
    void RecalculateNormals();
    void RecalculateNormals(Vertex_XYZUVNorm* pVerts);

    void SaveAsHeightmap(const char* filename);
#if MYFW_EDITOR
    void SaveAsMyMesh(const char* filename);
    virtual void AddAllVariablesToWatchPanel(CommandStack* pCommandStack) override;
    void LoadFromHeightmap(const char* filename);
#endif
    void LoadFromHeightmap();

    bool SnapToBounds(Vector3 start, const Vector3& dir, Vector3* pResult) const;
    bool FindCollisionPoint(const Vector3& currentPosition, const Vector3& start, const Vector3& dir, Vector2Int tile1, Vector2Int tile2, Vector3* pResult) const;

public:
    bool GetTileCoordsAtWorldXZ(const float x, const float z, Vector2Int* pLocalTile, Vector2* pPercIntoTile) const;
    bool GetHeightAtWorldXZ(const float x, const float z, float* pFloat) const;
    bool RayCast(bool rayIsInWorldSpace, Vector3 start, Vector3 end, Vector3* pResult) const;
    bool RayCastAtLocalHeight(bool rayIsInWorldSpace, Vector3 start, Vector3 end, float height, Vector3* pResultAtDesiredHeight, Vector3* pResultOnGround) const;

    // Editor tools, will all return true if they affect the vertices.
    bool Tool_Raise(Vector3 position, float amount, float radius, float softness, bool rebuild);
    bool Tool_Level(Vector3 position, float desiredHeight, float radius, float softness, bool rebuild);
};

#endif //__ComponentHeightmap_H__
