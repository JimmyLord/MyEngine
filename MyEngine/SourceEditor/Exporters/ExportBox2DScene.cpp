//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentSprite.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshOBJ.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshPrimitive.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointRevolute.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointPrismatic.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointWeld.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DCollisionObject.h"
#include "Core/EngineCore.h"

cJSON* ExportGameObject(cJSON* jGameObjectArray, GameObject* pGameObject)
{
    cJSON* jGameObject = 0;

    if( pGameObject->IsManaged() && pGameObject->IsFolder() == false )
    {
        jGameObject = cJSON_CreateObject();
        cJSON_AddStringToObject( jGameObject, "Name", pGameObject->GetName() );

        ComponentTransform* pTransform = pGameObject->GetTransform();
        Vector3 pos = pTransform->GetWorldPosition();
        Vector3 rot = pTransform->GetWorldRotation();
        Vector3 scale = pTransform->GetWorldScale();
        cJSONExt_AddFloatArrayToObject( jGameObject, "Pos", &pos.x, 3 );
        cJSONExt_AddFloatArrayToObject( jGameObject, "Rot", &rot.x, 3 );
        cJSONExt_AddFloatArrayToObject( jGameObject, "Scale", &scale.x, 3 );

        cJSON* jFlagsArray = cJSON_CreateArray();
        cJSON_AddItemToObject( jGameObject, "Flags", jFlagsArray );
        for( unsigned int i=0; i<32; i++ )
        {
            unsigned int flags = pGameObject->GetFlags();
            if( flags & (1<<i) )
            {
                cJSON* jFlag = cJSON_CreateString( g_pEngineCore->GetGameObjectFlagString( i ) );
                cJSON_AddItemToArray( jFlagsArray, jFlag );
            }
        }

        if( pGameObject->GetComponentCount() > 0 )
        {
            cJSON* jComponentArray = 0;

            for( unsigned int i=0; i<pGameObject->GetComponentCount(); i++ )
            {
                ComponentBase* pComponent = pGameObject->GetComponentByIndex( i );

                if( pComponent->IsA( "2DCollisionObjectComponent" ) ||
                    pComponent->IsA( "2DJoint-RevoluteComponent" ) ||
                    pComponent->IsA( "2DJoint-WeldComponent" ) ||
                    pComponent->IsA( "2DJoint-PrismaticComponent" ) ||
                    pComponent->IsA( "MeshPrimitiveComponent" ) ||
                    pComponent->IsA( "MeshOBJComponent" ) ||
                    pComponent->IsA( "SpriteComponent" ) ||
                    pComponent->IsA( "3DCollisionObjectComponent" ) ||
                    pComponent->IsA( "LightComponent" ) )
                {
                    if( jComponentArray == 0 )
                    {
                        jComponentArray = cJSON_CreateArray();
                        cJSON_AddItemToObject( jGameObject, "Components", jComponentArray );
                    }

                    cJSON* jComponent = pComponent->ExportAsJSONObject( false, true );
                    cJSON_AddItemToArray( jComponentArray, jComponent );

                    cJSON_DeleteItemFromObject( jComponent, "GOID" );
                    cJSON_DeleteItemFromObject( jComponent, "ID" );
                    cJSON_DeleteItemFromObject( jComponent, "Visible" );
                    cJSON_DeleteItemFromObject( jComponent, "Layers" );
                    cJSON_DeleteItemFromObject( jComponent, "SecondCollisionObject" );
                    cJSON_DeleteItemFromObject( jComponent, "Material" );
                    cJSON_DeleteItemFromObject( jComponent, "Materials" );
                    cJSON_DeleteItemFromObject( jComponent, "Tint" );
                    cJSON_DeleteItemFromObject( jComponent, "Size" );

                    if( Component2DJointRevolute* pJoint = dynamic_cast<Component2DJointRevolute*>( pComponent ) )
                    {
                        if( pJoint->m_pSecondCollisionObject )
                            cJSON_AddStringToObject( jComponent, "OtherGameObject", pJoint->m_pSecondCollisionObject->GetGameObject()->GetName() );
                    }
                    if( Component2DJointPrismatic* pJoint = dynamic_cast<Component2DJointPrismatic*>( pComponent ) )
                    {
                        if( pJoint->m_pSecondCollisionObject )
                            cJSON_AddStringToObject( jComponent, "OtherGameObject", pJoint->m_pSecondCollisionObject->GetGameObject()->GetName() );
                    }
                    if( Component2DJointWeld* pJoint = dynamic_cast<Component2DJointWeld*>( pComponent ) )
                    {
                        if( pJoint->m_pSecondCollisionObject )
                            cJSON_AddStringToObject( jComponent, "OtherGameObject", pJoint->m_pSecondCollisionObject->GetGameObject()->GetName() );
                    }

                    MaterialDefinition* pMaterial = 0;

                    if( pComponent->IsA( "SpriteComponent" ) )
                    {
                        ComponentSprite* pSprite = (ComponentSprite*)pComponent;
                        pMaterial = pSprite->GetMaterial( 0 );
                    }

                    if( pComponent->IsA( "MeshPrimitiveComponent" ) )
                    {
                        ComponentMeshPrimitive* pMesh = (ComponentMeshPrimitive*)pComponent;
                        pMaterial = pMesh->GetMaterial( 0 );
                    }

                    if( pComponent->IsA( "MeshOBJComponent" ) )
                    {
                        ComponentMeshOBJ* pMesh = (ComponentMeshOBJ*)pComponent;
                        pMaterial = pMesh->GetMaterial( 0 );

                        if( pMesh->m_pMesh && pMesh->m_pMesh->GetFile() )
                            cJSON_AddStringToObject( jComponent, "OBJFilename", pMesh->m_pMesh->GetFile()->GetFilenameWithoutExtension() );
                    }

                    if( pComponent->IsA( "3DCollisionObjectComponent" ) )
                    {
                        Component3DCollisionObject* p3DComponent = (Component3DCollisionObject*)pComponent;
                        MyMesh* pMesh = p3DComponent->GetMesh();

                        if( pMesh )
                        {
                            cJSON_AddStringToObject( jComponent, "OBJFilename", pMesh->GetFile()->GetFilenameWithoutExtension() );
                        }
                    }

                    if( pMaterial )
                    {
                        if( pMaterial )
                        {
                            const char* name = pMaterial->GetTextureColor()->GetFilename();
                            cJSON_AddStringToObject( jComponent, "Material", pMaterial->GetName() );
                        }

                        if( pMaterial->GetTextureColor() )
                        {
                            const char* name = pMaterial->GetTextureColor()->GetFilename();
                            cJSON_AddStringToObject( jComponent, "Texture", name );
                        }
                                            
                        if( pMaterial->GetShader() )
                        {
                            const char* name = pMaterial->GetShader()->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
                            cJSON_AddStringToObject( jComponent, "Shader", name );
                        }

                        ColorByte color = pMaterial->GetColorDiffuse();
                        cJSONExt_AddUnsignedCharArrayToObject( jComponent, "Color", &color.r, 4 );
                    }
                }
            }
        }
    }

    // export children
    for( GameObject* pChildGameObject = pGameObject->GetChildList()->GetHead(); pChildGameObject != 0; pChildGameObject = pChildGameObject->GetNext() )
    {
        cJSON* jChildGameObject = ExportGameObject( jGameObjectArray, pChildGameObject );
        cJSON_AddItemToArray( jGameObjectArray, jChildGameObject );
    }

    return jGameObject;
}

char* ExportBox2DSceneToJSON(ComponentSystemManager* pComponentSystemManager, SceneID sceneid)
{
    cJSON* jRoot = cJSON_CreateObject();
    
    cJSON* jGameObjectArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jRoot, "GameObjects", jGameObjectArray );

    bool savingallscenes = (sceneid == SCENEID_AllScenes);

    // add the game objects and their transform components.
    {
        for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                continue;

            SceneInfo* pSceneInfo = &pComponentSystemManager->m_pSceneInfoMap[i];

            GameObject* first = pSceneInfo->m_GameObjects.GetHead();
            if( first && ( first->GetSceneID() == sceneid || savingallscenes ) )
            {
                for( GameObject* pGameObject = first; pGameObject; pGameObject = pGameObject->GetNext() )
                {
                    cJSON* jGameObject = ExportGameObject( jGameObjectArray, pGameObject );
                    cJSON_AddItemToArray( jGameObjectArray, jGameObject );
                }
            }
        }
    }

    char* savestring = cJSON_Print( jRoot );
    cJSON_Delete( jRoot );

    return savestring;
}