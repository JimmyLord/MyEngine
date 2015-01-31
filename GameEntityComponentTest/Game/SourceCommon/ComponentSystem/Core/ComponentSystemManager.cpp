//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentSystemManager* g_pComponentSystemManager = 0;

ComponentSystemManager::ComponentSystemManager(ComponentTypeManager* typemanager)
{
    g_pComponentSystemManager = this;

    m_pComponentTypeManager = typemanager;

    m_NextGameObjectID = 1;
    m_NextComponentID = 1;

#if MYFW_USING_WX
    // Add click callbacks to the root of the objects tree
    g_pPanelObjectList->SetTreeRootData( this, ComponentSystemManager::StaticOnLeftClick, ComponentSystemManager::StaticOnRightClick );
#endif //MYFW_USING_WX
}

ComponentSystemManager::~ComponentSystemManager()
{
    while( m_GameObjects.GetHead() )
        delete m_GameObjects.RemHead();

    while( m_Files.GetHead() )
        delete m_Files.RemHead();

    while( m_ComponentsData.GetHead() )
        delete m_ComponentsData.RemHead();

    while( m_ComponentsCamera.GetHead() )
        delete m_ComponentsCamera.RemHead();    

    while( m_ComponentsInputHandler.GetHead() )
        delete m_ComponentsInputHandler.RemHead();    
    
    while( m_ComponentsUpdateable.GetHead() )
        delete m_ComponentsUpdateable.RemHead();

    while( m_ComponentsRenderable.GetHead() )
        delete m_ComponentsRenderable.RemHead();

    SAFE_DELETE( m_pComponentTypeManager );
    
    g_pComponentSystemManager = 0;
}

void ComponentSystemManager::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSystemManager>( "ComponentSystemManager" )
            .addFunction( "CreateGameObject", &ComponentSystemManager::CreateGameObject )
            .addFunction( "DeleteGameObject", &ComponentSystemManager::DeleteGameObject )
            .addFunction( "CopyGameObject", &ComponentSystemManager::CopyGameObject )
            .addFunction( "FindGameObjectByName", &ComponentSystemManager::FindGameObjectByName )
        .endClass();
}

#if MYFW_USING_WX
void ComponentSystemManager::OnLeftClick(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();
}

void ComponentSystemManager::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    menu.Append( 0, "Add Game Object" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentSystemManager::OnPopupClick );

    // blocking call. // should delete all categorymenu's new'd above when done.
 	g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentSystemManager::OnPopupClick(wxEvent &evt)
{
    int id = evt.GetId();
    if( id == 0 )
    {
        GameObject* pGameObject = g_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "New Game Object" );
    }
}
#endif //MYFW_USING_WX

char* ComponentSystemManager::SaveSceneToJSON()
{
    cJSON* root = cJSON_CreateObject();
    cJSON* filearray = cJSON_CreateArray();
    cJSON* gameobjectarray = cJSON_CreateArray();
    cJSON* transformarray = cJSON_CreateArray();
    cJSON* componentarray = cJSON_CreateArray();

    cJSON_AddItemToObject( root, "Files", filearray );
    cJSON_AddItemToObject( root, "GameObjects", gameobjectarray );
    cJSON_AddItemToObject( root, "Transforms", transformarray );
    cJSON_AddItemToObject( root, "Components", componentarray );

    // add the files used.
    {
        MyFileObject* pFile = g_pFileManager->GetFirstFileLoaded();
        while( pFile != 0 )
        {
            cJSON_AddItemToArray( filearray, cJSON_CreateString( pFile->m_FullPath ) );
            pFile = (MyFileObject*)pFile->GetNext();
        }

        pFile = g_pFileManager->GetFirstFileStillLoading();
        while( pFile != 0 )
        {
            cJSON_AddItemToArray( filearray, cJSON_CreateString( pFile->m_FullPath ) );
            pFile = (MyFileObject*)pFile->GetNext();
        }
    }

    // add the game objects and their transform components.
    {
        for( CPPListNode* pNode = m_GameObjects.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            GameObject* pGameObject = (GameObject*)pNode;
            if( pGameObject->IsManaged() )
                cJSON_AddItemToArray( gameobjectarray, pGameObject->ExportAsJSONObject() );
        }

        for( CPPListNode* pNode = m_GameObjects.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            GameObject* pGameObject = (GameObject*)pNode;
            if( pGameObject->IsManaged() )
            {
                ComponentBase* pComponent = pGameObject->m_pComponentTransform;
                cJSON_AddItemToArray( transformarray, pComponent->ExportAsJSONObject() );
            }
        }
    }

    // Add each of the component types, don't save components of unmanaged objects
    {
        for( CPPListNode* pNode = m_ComponentsCamera.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            if( pComponent->m_pGameObject->IsManaged() )
                cJSON_AddItemToArray( componentarray, pComponent->ExportAsJSONObject() );
        }

        for( CPPListNode* pNode = m_ComponentsInputHandler.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            if( pComponent->m_pGameObject->IsManaged() )
                cJSON_AddItemToArray( componentarray, pComponent->ExportAsJSONObject() );
        }

        for( CPPListNode* pNode = m_ComponentsUpdateable.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            if( pComponent->m_pGameObject->IsManaged() )
                cJSON_AddItemToArray( componentarray, pComponent->ExportAsJSONObject() );
        }

        for( CPPListNode* pNode = m_ComponentsRenderable.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            if( pComponent->m_pGameObject->IsManaged() )
                cJSON_AddItemToArray( componentarray, pComponent->ExportAsJSONObject() );
        }

        for( CPPListNode* pNode = m_ComponentsData.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            if( pComponent->m_pGameObject->IsManaged() )
                cJSON_AddItemToArray( componentarray, pComponent->ExportAsJSONObject() );
        }
    }

    char* savestring = cJSON_Print( root );
    cJSON_Delete(root);

    return savestring;
}

bool ComponentSystemManager::IsFileUsedByScene(const char* fullpath, unsigned int sceneid)
{
    for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        FileInfo* pFile = (FileInfo*)pNode;
        
        if( strcmp( pFile->m_pFile->m_FullPath, fullpath ) == 0 && pFile->m_SceneID == sceneid )
            return true;
    }

    return false;
}

void ComponentSystemManager::LoadDatafile(const char* fullpath, unsigned int sceneid)
{
    // if the file is already tagged as being used by this scene, don't request/addref it.
    if( IsFileUsedByScene( fullpath, sceneid ) == false )
    {
        // each scene loaded will add ref's to the file.
        MyFileObject* pFile = 0;

        size_t len = strlen( fullpath );
        if( strcmp( &fullpath[len-4], ".png" ) == 0 )
        {
            TextureDefinition* pTexture = g_pTextureManager->CreateTexture( fullpath );
            pFile = pTexture->m_pFile;
            pFile->AddRef();
        }
        else
        {
            pFile = g_pFileManager->RequestFile( fullpath );
        }

        // store pFile so we can free it afterwards.
        FileInfo* pFileInfo = MyNew FileInfo();
        pFileInfo->m_pFile = pFile;
        pFileInfo->m_SceneID = sceneid;
        m_Files.AddTail( pFileInfo );

        // if we're loading an .obj file, create a mesh.
        if( strcmp( pFile->m_ExtensionWithDot, ".obj" ) == 0 )
        {
            pFileInfo->m_pMesh = MyNew MyMesh();
            pFileInfo->m_pMesh->CreateFromOBJFile( pFile );
        }

        // if we're loading an .glsl file, create a ShaderGroup.
        if( strcmp( pFile->m_ExtensionWithDot, ".glsl" ) == 0 )
        {
            pFileInfo->m_pShaderGroup = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, pFile->m_FilenameWithoutExtension );
            pFileInfo->m_pShaderGroup->SetFileForAllPasses( pFile );
        }
    }
}

void ComponentSystemManager::LoadSceneFromJSON(const char* jsonstr, unsigned int sceneid)
{
    cJSON* root = cJSON_Parse( jsonstr );

    if( root == 0 )
        return;

    cJSON* filearray = cJSON_GetObjectItem( root, "Files" );
    cJSON* gameobjectarray = cJSON_GetObjectItem( root, "GameObjects" );
    cJSON* transformarray = cJSON_GetObjectItem( root, "Transforms" );
    cJSON* componentarray = cJSON_GetObjectItem( root, "Components" );

    // request all files used by scene.
    if( filearray )
    {
        for( int i=0; i<cJSON_GetArraySize( filearray ); i++ )
        {
            cJSON* fileobj = cJSON_GetArrayItem( filearray, i );

            LoadDatafile( fileobj->valuestring, sceneid );
        }
    }

    // create/init all the game objects
    for( int i=0; i<cJSON_GetArraySize( gameobjectarray ); i++ )
    {
        cJSON* gameobj = cJSON_GetArrayItem( gameobjectarray, i );

        unsigned int id;
        cJSONExt_GetUnsignedInt( gameobj, "ID", &id );

        // find an existing game object with the same id or create a new one.
        GameObject* pGameObject = FindGameObjectByID( id );
        if( pGameObject ) { assert( pGameObject->GetSceneID() == sceneid ); } // TODO: get the object from the right scene if multiple scenes are loaded.

        if( pGameObject == 0 )        
            pGameObject = CreateGameObject();

        pGameObject->ImportFromJSONObject( gameobj, sceneid );

        if( pGameObject->GetID() > m_NextGameObjectID )
            m_NextGameObjectID += 1;
    }

    // setup all the game object transforms
    for( int i=0; i<cJSON_GetArraySize( transformarray ); i++ )
    {
        cJSON* transformobj = cJSON_GetArrayItem( transformarray, i );
        
        unsigned int goid = 0;
        cJSONExt_GetUnsignedInt( transformobj, "GOID", &goid );
        assert( goid > 0 );

        GameObject* pGameObject = FindGameObjectByID( goid );
        assert( pGameObject );

        if( pGameObject )
        {
            pGameObject->SetID( goid );
            pGameObject->m_pComponentTransform->ImportFromJSONObject( transformobj, sceneid );

            if( goid >= m_NextGameObjectID )
                m_NextGameObjectID = goid + 1;
        }
    }

    // create/init all the other components
    for( int i=0; i<cJSON_GetArraySize( componentarray ); i++ )
    {
        cJSON* componentobj = cJSON_GetArrayItem( componentarray, i );
        
        unsigned int id = 0;
        cJSONExt_GetUnsignedInt( componentobj, "GOID", &id );
        assert( id > 0 );
        GameObject* pGameObject = FindGameObjectByID( id );
        assert( pGameObject );

        cJSON* typeobj = cJSON_GetObjectItem( componentobj, "Type" );
        assert( typeobj );
        int type = -1;
        if( typeobj )
            type = g_pComponentTypeManager->GetTypeByName( typeobj->valuestring );

        if( type == -1 )
        {
            LOGError( LOGTag, "Unknown component in scene file: %s\n", typeobj->valuestring );
#if _DEBUG
            assert( false );
#endif
        }
        else
        {
            unsigned int id;
            cJSONExt_GetUnsignedInt( componentobj, "ID", &id );

            ComponentBase* pComponent = FindComponentByID( id );
            if( pComponent ) { assert( pComponent->GetSceneID() == sceneid ); } // TODO: get the object from the right scene if multiple scenes are loaded.
    
            if( pComponent == 0 )
                pComponent = pGameObject->AddNewComponent( type, sceneid );

            pComponent->ImportFromJSONObject( componentobj, sceneid );
            
            pComponent->SetID( id );
            if( id >= m_NextComponentID )
                m_NextComponentID = id + 1;
        }
    }

    cJSON_Delete( root );

    SyncAllRigidBodiesToObjectTransforms();
}

void ComponentSystemManager::SyncAllRigidBodiesToObjectTransforms()
{
    for( CPPListNode* pNode = m_ComponentsUpdateable.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        if( ((ComponentBase*)pNode)->m_Type == ComponentType_CollisionObject )
        {
            ComponentCollisionObject* pComponent = (ComponentCollisionObject*)pNode;

            pComponent->SyncRigidBodyToTransform();
        }
    }
}

void ComponentSystemManager::UnloadScene(bool clearunmanagedcomponents, unsigned int sceneidtoclear)
{
    m_NextGameObjectID = 1;
    m_NextComponentID = 1;

    // Remove all components, except ones attached to unmanaged game objects(if wanted)
    {
        for( CPPListNode* pNode = m_ComponentsCamera.GetHead(); pNode;  )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            pNode = pNode->GetNext();
            if( (pComponent->m_pGameObject->IsManaged() || clearunmanagedcomponents) &&
                (sceneidtoclear == UINT_MAX || pComponent->GetSceneID() == sceneidtoclear) )
            {
                DeleteComponent( pComponent );
            }
        }

        for( CPPListNode* pNode = m_ComponentsInputHandler.GetHead(); pNode;  )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            pNode = pNode->GetNext();
            if( (pComponent->m_pGameObject->IsManaged() || clearunmanagedcomponents) &&
                (sceneidtoclear == UINT_MAX || pComponent->GetSceneID() == sceneidtoclear) )
            {
                DeleteComponent( pComponent );
            }
        }

        for( CPPListNode* pNode = m_ComponentsUpdateable.GetHead(); pNode;  )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            pNode = pNode->GetNext();
            if( (pComponent->m_pGameObject->IsManaged() || clearunmanagedcomponents) &&
                (sceneidtoclear == UINT_MAX || pComponent->GetSceneID() == sceneidtoclear) )
            {
                DeleteComponent( pComponent );
            }
        }

        for( CPPListNode* pNode = m_ComponentsRenderable.GetHead(); pNode;  )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            pNode = pNode->GetNext();
            if( (pComponent->m_pGameObject->IsManaged() || clearunmanagedcomponents) &&
                (sceneidtoclear == UINT_MAX || pComponent->GetSceneID() == sceneidtoclear) )
            {
                DeleteComponent( pComponent );
            }
        }

        for( CPPListNode* pNode = m_ComponentsData.GetHead(); pNode;  )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            pNode = pNode->GetNext();
            if( (pComponent->m_pGameObject->IsManaged() || clearunmanagedcomponents) &&
                (sceneidtoclear == UINT_MAX || pComponent->GetSceneID() == sceneidtoclear) )
            {
                DeleteComponent( pComponent );
            }
        }
    }

    // delete all game objects.
    {
        for( CPPListNode* pNode = m_GameObjects.GetHead(); pNode;  )
        {
            GameObject* pGameObject = (GameObject*)pNode;

            pNode = pNode->GetNext();

            if( (sceneidtoclear == UINT_MAX || pGameObject->GetSceneID() == sceneidtoclear) )
            {
                DeleteGameObject( pGameObject, true );
            }
        }
    }

    // release any file ref's added by this scene.
    for( CPPListNode* pNode = m_Files.GetHead(); pNode;  )
    {
        FileInfo* pFile = (FileInfo*)pNode;
        pNode = pNode->GetNext();

        if( pFile->m_SceneID == sceneidtoclear || pFile->m_SceneID == UINT_MAX )
            delete pFile;
    }
}

GameObject* ComponentSystemManager::CreateGameObject(bool manageobject)
{
    GameObject* pGameObject = MyNew GameObject( manageobject );
    pGameObject->SetID( m_NextGameObjectID );
    m_NextGameObjectID++;

    if( manageobject )
        m_GameObjects.AddTail( pGameObject );

    return pGameObject;
}

void ComponentSystemManager::UnmanageGameObject(GameObject* pObject)
{
    assert( pObject && pObject->IsManaged() == true );

    // remove all components from their respective component lists.
    for( unsigned int i=0; i<pObject->m_Components.Count(); i++ )
    {
        // remove from list and clear CPPListNode prev/next
        pObject->m_Components[i]->Remove();
        pObject->m_Components[i]->Prev = 0;
        pObject->m_Components[i]->Next = 0;
    }

    pObject->SetManaged( false );
}

void ComponentSystemManager::ManageGameObject(GameObject* pObject)
{
    assert( pObject && pObject->IsManaged() == false );

    // add all the gameobject's components back into the component lists.
    for( unsigned int i=0; i<pObject->m_Components.Count(); i++ )
    {
        AddComponent( pObject->m_Components[i] );
    }

    pObject->SetManaged( true );
}

void ComponentSystemManager::DeleteGameObject(GameObject* pObject, bool deletecomponents)
{
    if( deletecomponents )
    {
        while( pObject->m_Components.Count() )
        {
            ComponentBase* pComponent = pObject->m_Components.RemoveIndex( 0 );
            delete pComponent;
        }
    }

    SAFE_DELETE( pObject );
}

GameObject* ComponentSystemManager::EditorCopyGameObject(GameObject* pObject)
{
    // add the undo action, don't reperform it, it's done above.
    EditorCommand_CopyGameObject* pCommand = MyNew EditorCommand_CopyGameObject( pObject );
    g_pGameMainFrame->m_pCommandStack->Do( pCommand );

    return pCommand->GetCreatedObject();
}

GameObject* ComponentSystemManager::CopyGameObject(GameObject* pObject, const char* newname)
{
    GameObject* pNewObject = CreateGameObject();
    pNewObject->SetName( newname );
    pNewObject->SetSceneID( pObject->GetSceneID() );

    *pNewObject->m_pComponentTransform = *pObject->m_pComponentTransform;

    for( unsigned int i=0; i<pObject->m_Components.Count(); i++ )
    {
        ComponentBase* pComponent = 0;

        if( ((GameEntityComponentTest*)g_pGameCore)->m_EditorMode )
            pComponent = pNewObject->AddNewComponent( pObject->m_Components[i]->m_Type, pNewObject->GetSceneID() );
        else
            pComponent = pNewObject->AddNewComponent( pObject->m_Components[i]->m_Type, 0 );

        pComponent->CopyFromSameType_Dangerous( pObject->m_Components[i] );        
    }

    return pNewObject;
}

GameObject* ComponentSystemManager::FindGameObjectByID(unsigned int id)
{
    for( CPPListNode* node = m_GameObjects.GetHead(); node != 0; node = node->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)node;

        if( pGameObject->GetID() == id )
            return pGameObject;
    }

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByName(const char* name)
{
    for( CPPListNode* node = m_GameObjects.GetHead(); node != 0; node = node->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)node;

        if( strcmp( pGameObject->GetName(), name ) == 0 )
            return pGameObject;
    }

    return 0;
}

ComponentCamera* ComponentSystemManager::GetFirstCamera()
{
    for( CPPListNode* node = m_ComponentsCamera.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pCamera = (ComponentCamera*)node;

        // skip unmanaged cameras. (editor cam)
        if( pCamera->m_pGameObject->IsManaged() == true )
        {
            assert( pCamera->m_Type == ComponentType_Camera );

            return pCamera;
        }
    }

    return 0;
}

ComponentBase* ComponentSystemManager::AddComponent(ComponentBase* pComponent)
{
    switch( pComponent->m_BaseType )
    {
    case BaseComponentType_Data:
        m_ComponentsData.AddTail( pComponent );
        break;

    case BaseComponentType_Camera:
        m_ComponentsCamera.AddTail( pComponent );
        break;

    case BaseComponentType_InputHandler:
        m_ComponentsInputHandler.AddTail( pComponent );
        break;

    case BaseComponentType_Updateable:
        m_ComponentsUpdateable.AddTail( pComponent );
        break;

    case BaseComponentType_Renderable:
        m_ComponentsRenderable.AddTail( pComponent );
        break;

    case BaseComponentType_None:
        assert( false ); // shouldn't happen.
        break;
    }

    return pComponent;
}

void ComponentSystemManager::DeleteComponent(ComponentBase* pComponent)
{
    if( pComponent->m_pGameObject )
    {
        pComponent = pComponent->m_pGameObject->RemoveComponent( pComponent );
        if( pComponent == 0 )
            return;
    }

#if MYFW_USING_WX
    g_pPanelObjectList->RemoveObject( pComponent );
#endif
    pComponent->Remove();
    SAFE_DELETE( pComponent );
}

ComponentBase* ComponentSystemManager::FindComponentByID(unsigned int id)
{
    for( CPPListNode* pNode = m_ComponentsCamera.GetHead(); pNode;  )
    {
        ComponentBase* pComponent = (ComponentBase*)pNode;
        pNode = pNode->GetNext();
        if( pComponent->GetID() == id )
            return pComponent;
    }

    for( CPPListNode* pNode = m_ComponentsInputHandler.GetHead(); pNode;  )
    {
        ComponentBase* pComponent = (ComponentBase*)pNode;
        pNode = pNode->GetNext();
        if( pComponent->GetID() == id )
            return pComponent;
    }

    for( CPPListNode* pNode = m_ComponentsUpdateable.GetHead(); pNode;  )
    {
        ComponentBase* pComponent = (ComponentBase*)pNode;
        pNode = pNode->GetNext();
        if( pComponent->GetID() == id )
            return pComponent;
    }

    for( CPPListNode* pNode = m_ComponentsRenderable.GetHead(); pNode;  )
    {
        ComponentBase* pComponent = (ComponentBase*)pNode;
        pNode = pNode->GetNext();
        if( pComponent->GetID() == id )
            return pComponent;
    }

    for( CPPListNode* pNode = m_ComponentsData.GetHead(); pNode;  )
    {
        ComponentBase* pComponent = (ComponentBase*)pNode;
        pNode = pNode->GetNext();
        if( pComponent->GetID() == id )
            return pComponent;
    }

    return 0;
}

void ComponentSystemManager::Tick(double TimePassed)
{
    // update all game objects.
    for( CPPListNode* node = m_ComponentsUpdateable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentUpdateable* pComponent = (ComponentUpdateable*)node;

        if( pComponent->m_BaseType == BaseComponentType_Updateable )
        {
            pComponent->Tick( TimePassed );
        }
    }

    // update all cameras after game objects are updated.
    for( CPPListNode* node = m_ComponentsCamera.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pComponent = (ComponentCamera*)node;

        if( pComponent->m_BaseType == BaseComponentType_Camera )
        {
            pComponent->Tick( TimePassed );
        }
    }
}

void ComponentSystemManager::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    for( CPPListNode* node = m_ComponentsCamera.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pCamera = (ComponentCamera*)node;

        if( pCamera->m_BaseType == BaseComponentType_Camera )
        {
            // TODO: fix this hack, don't resize unmanaged cams (a.k.a. editor camera)
            if( pCamera->m_pGameObject->IsManaged() == true )
            {
                pCamera->OnSurfaceChanged( startx, starty, width, height, desiredaspectwidth, desiredaspectheight );
            }
        }
    }
}

void ComponentSystemManager::OnDrawFrame()
{
    for( CPPListNode* node = m_ComponentsCamera.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pComponent = (ComponentCamera*)node;

        if( pComponent->m_BaseType == BaseComponentType_Camera && pComponent->m_Enabled == true )
        {
            pComponent->OnDrawFrame();
        }
    }
}

void ComponentSystemManager::OnDrawFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    for( CPPListNode* node = m_ComponentsRenderable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentRenderable* pComponent = (ComponentRenderable*)node;

        if( pComponent->m_BaseType == BaseComponentType_Renderable )
        {
            if( pComponent->m_LayersThisExistsOn & pCamera->m_LayersToRender )
            {
                if( pComponent->m_Visible )
                {
                    pComponent->Draw( pMatViewProj, pShaderOverride );
                }
            }
        }
    }
}

void ComponentSystemManager::DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderGroup)
{
    Shader_Base* pShader = (Shader_Base*)pShaderGroup->GlobalPass();
    if( pShader->ActivateAndProgramShader() )
    {
        for( CPPListNode* node = m_ComponentsRenderable.GetHead(); node != 0; node = node->GetNext() )
        {
            ComponentRenderable* pComponent = (ComponentRenderable*)node;

            if( pComponent->m_BaseType == BaseComponentType_Renderable )
            {
                if( pComponent->m_LayersThisExistsOn & pCamera->m_LayersToRender )
                {
                    ColorByte tint( 0, 0, 0, 0 );

                    unsigned int id = pComponent->m_pGameObject->GetID();

                    if( 1 )                 tint.r = id%256;
                    if( id > 256 )          tint.g = (id>>8)%256;
                    if( id > 256*256 )      tint.b = (id>>16)%256;
                    if( id > 256*256*256 )  tint.a = (id>>24)%256;

                    pShader->ProgramTint( tint );

                    if( pComponent->m_Visible )
                    {
                        pComponent->Draw( pMatViewProj, pShaderGroup );
                    }
                }
            }
        }

        pShader->DeactivateShader();
    }
}

void ComponentSystemManager::OnPlay()
{
    for( CPPListNode* node = m_ComponentsUpdateable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentBase* pComponent = (ComponentBase*)node;
        pComponent->OnPlay();
    }
}

void ComponentSystemManager::OnStop()
{
    for( CPPListNode* node = m_ComponentsUpdateable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentBase* pComponent = (ComponentBase*)node;
        pComponent->OnStop();
    }
}

bool ComponentSystemManager::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    for( CPPListNode* node = m_ComponentsInputHandler.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)node;

        if( pComponent->m_BaseType == BaseComponentType_InputHandler )
        {
            if( pComponent->OnTouch( action, id, x, y, pressure, size ) == true )
                return true;
        }
    }

    // send input to all the scripts.
    for( CPPListNode* node = m_ComponentsUpdateable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentLuaScript* pComponent = dynamic_cast<ComponentLuaScript*>( node );

        if( pComponent )
        {
            if( pComponent->OnTouch( action, id, x, y, pressure, size ) == true )
                return true;
        }
    }

    return false;
}

bool ComponentSystemManager::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    for( CPPListNode* node = m_ComponentsInputHandler.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)node;

        if( pComponent->m_BaseType == BaseComponentType_InputHandler )
        {
            if( pComponent->OnButtons( action, id ) == true )
                return true;
        }
    }

    // send input to all the scripts.
    for( CPPListNode* node = m_ComponentsUpdateable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentLuaScript* pComponent = dynamic_cast<ComponentLuaScript*>( node );

        if( pComponent )
        {
            if( pComponent->OnButtons( action, id ) == true )
                return true;
        }
    }

    return false;
}
