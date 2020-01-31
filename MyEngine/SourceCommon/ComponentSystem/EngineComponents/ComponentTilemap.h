//
// Copyright (c) 2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentTilemap_H__
#define __ComponentTilemap_H__

#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"

class ComponentTilemap : public ComponentMesh
{
    friend class EditorDocument_Tilemap;
    //friend class EditorCommand_Tilemap_Raise;
    //friend class EditorCommand_Tilemap_FullBackup;
    //friend class Job_CalculateNormals;

    typedef unsigned int TileIndex;

private:
    // Component Variable List.
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentTilemap );

protected:
    Vector2 m_Size;
    Vector2Int m_TileCount;
    TileIndex* m_Tiles;

    MyFileObject* m_pTilemapFile;
    Vector2Int m_TilemapFileSize;
    bool m_WaitingForTilemapFileToFinishLoading;

    TextureDefinition* m_pTilemapTexture;
    Vector2Int m_TilemapTextureSize;
    bool m_WaitingForTextureFileToFinishLoading;

public:
    ComponentTilemap(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentTilemap();
    SetClassnameWith2Parents( "TilemapComponent", ComponentMesh, ComponentRenderable );

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luaState);
#endif //MYFW_USING_LUA

    //virtual cJSON* ExportAsJSONObject(bool saveSceneID, bool saveID);
    //virtual void ImportFromJSONObject(cJSON* jComponent, SceneID sceneID);
    virtual void FinishImportingFromJSONObject(cJSON* jComponent) override;

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentTilemap&)*pObject; }
    ComponentTilemap& operator=(const ComponentTilemap& other);

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

    void OnButtonEditTilemap();
#endif //MYFW_EDITOR

    void SetTilemapFile(MyFileObject* pFile);
    void UnregisterTilemapFileLoadingCallbacks(bool force);
    static void StaticOnFileFinishedLoadingTilemapFile(void* pObjectPtr, MyFileObject* pFile) { ((ComponentTilemap*)pObjectPtr)->OnFileFinishedLoadingTilemapFile( pFile ); }
    void OnFileFinishedLoadingTilemapFile(MyFileObject* pFile);

    void SetTilemapTexture(TextureDefinition* pTexture);
    void UnregisterTilemapTextureLoadingCallbacks(bool force);
    static void StaticOnFileFinishedLoadingTilemapTexture(void* pObjectPtr, MyFileObject* pFile) { ((ComponentTilemap*)pObjectPtr)->OnFileFinishedLoadingTilemapTexture( pFile ); }
    void OnFileFinishedLoadingTilemapTexture(MyFileObject* pFile);

    void CreateTilemap();
    bool GenerateTilemapMesh(bool createFromTexture, bool sizeChanged, bool rebuildNormals);

    void SaveAsTilemap(const char* filename);
#if MYFW_EDITOR
    void SaveAsMyMesh(const char* filename);
    virtual void AddAllVariablesToWatchPanel(CommandStack* pCommandStack) override;
    void LoadFromTilemap(const char* filename);
#endif
    void LoadFromTilemap();

public:
    bool GetTileCoordsAtWorldXZ(const float x, const float z, Vector2Int* pLocalTile, Vector2* pPercIntoTile) const;
    bool AreTileCoordsInBounds(Vector2Int tileCoords);
    Vector3 GetWorldPosAtTileCoords(Vector2Int tileCoords, bool centerOfTile) const;
    bool RayCast(bool rayIsInWorldSpace, Vector3 start, Vector3 end, Vector3* pResult) const;

    // Editor tools, will all return true if they affect the vertices.
    bool Tool_Raise(Vector3 position, float amount, float radius, float softness, bool rebuild);
    bool Tool_Level(Vector3 position, float desiredHeight, float radius, float softness, bool rebuild);
};

#endif //__ComponentTilemap_H__
