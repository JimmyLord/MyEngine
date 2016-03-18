//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

cJSON* ExportGameObject(cJSON* jGameObjectArray, GameObject* pGameObject)
{
    cJSON* jGameObject = 0;

    if( pGameObject->IsManaged() && pGameObject->IsFolder() == false )
    {
        jGameObject = cJSON_CreateObject();
        cJSON_AddStringToObject( jGameObject, "Name", pGameObject->GetName() );

        ComponentTransform* pTransform = pGameObject->m_pComponentTransform;
        Vector3 pos = pTransform->GetWorldPosition();
        Vector3 rot = pTransform->GetWorldRotation();
        Vector3 scale = pTransform->GetWorldScale();
        cJSONExt_AddFloatArrayToObject( jGameObject, "Pos", &pos.x, 3 );
        cJSONExt_AddFloatArrayToObject( jGameObject, "Rot", &rot.x, 3 );
        cJSONExt_AddFloatArrayToObject( jGameObject, "Scale", &scale.x, 3 );

        if( pGameObject->m_Components.Count() > 0 )
        {
            cJSON* jComponentArray = 0;

            for( unsigned int i=0; i<pGameObject->m_Components.Count(); i++ )
            {
                ComponentBase* pComponent = pGameObject->m_Components[i];

                if( pComponent->IsA( "2DCollisionObjectComponent" ) ||
                    pComponent->IsA( "2DJoint-Revolute" ) ||
                    pComponent->IsA( "2DJoint-Weld" ) ||
                    pComponent->IsA( "2DJoint-Prismatic" ) ||
                    pComponent->IsA( "MeshPrimitiveComponent" ) ||
                    pComponent->IsA( "SpriteComponent" ) )
                {
                    if( jComponentArray == 0 )
                    {
                        jComponentArray = cJSON_CreateArray();
                        cJSON_AddItemToObject( jGameObject, "Components", jComponentArray );
                    }

                    cJSON* jComponent = pComponent->ExportAsJSONObject( false );
                    cJSON_AddItemToArray( jComponentArray, jComponent );

                    cJSON_DeleteItemFromObject( jComponent, "GOID" );
                    cJSON_DeleteItemFromObject( jComponent, "ID" );
                    cJSON_DeleteItemFromObject( jComponent, "Visible" );
                    cJSON_DeleteItemFromObject( jComponent, "Layers" );
                    cJSON_DeleteItemFromObject( jComponent, "SecondCollisionObject" );
                    cJSON_DeleteItemFromObject( jComponent, "Material" );
                    cJSON_DeleteItemFromObject( jComponent, "Materials" );

                    if( Component2DJointRevolute* pJoint = dynamic_cast<Component2DJointRevolute*>(pComponent) )
                    {
                        if( pJoint->m_pSecondCollisionObject )
                            cJSON_AddStringToObject( jComponent, "OtherGameObject", pJoint->m_pSecondCollisionObject->m_pGameObject->GetName() );
                    }
                    if( Component2DJointPrismatic* pJoint = dynamic_cast<Component2DJointPrismatic*>(pComponent) )
                    {
                        if( pJoint->m_pSecondCollisionObject )
                            cJSON_AddStringToObject( jComponent, "OtherGameObject", pJoint->m_pSecondCollisionObject->m_pGameObject->GetName() );
                    }
                    if( Component2DJointWeld* pJoint = dynamic_cast<Component2DJointWeld*>(pComponent) )
                    {
                        if( pJoint->m_pSecondCollisionObject )
                            cJSON_AddStringToObject( jComponent, "OtherGameObject", pJoint->m_pSecondCollisionObject->m_pGameObject->GetName() );
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

                    if( pMaterial )
                    {
                        if( pMaterial->GetTextureColor() )
                        {
                            char* name = pMaterial->GetTextureColor()->m_Filename;
                            cJSON_AddStringToObject( jComponent, "Texture", name );
                        }
                                            
                        if( pMaterial->GetShader() )
                        {
                            char* name = pMaterial->GetShader()->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
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
    for( CPPListNode* pNode = pGameObject->GetChildList()->GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        GameObject* pChildGameObject = (GameObject*)pNode;
        cJSON* jChildGameObject = ExportGameObject( jGameObjectArray, pChildGameObject );
        cJSON_AddItemToArray( jGameObjectArray, jChildGameObject );
    }

    return jGameObject;
}

char* ExportBox2DSceneToJSON(ComponentSystemManager* pComponentSystemManager, unsigned int sceneid)
{
    cJSON* jRoot = cJSON_CreateObject();
    
    cJSON* jGameObjectArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jRoot, "GameObjects", jGameObjectArray );

    bool savingallscenes = (sceneid == UINT_MAX);

    // add the game objects and their transform components.
    {
        for( unsigned int i=0; i<ComponentSystemManager::MAX_SCENES_LOADED; i++ )
        {
            if( pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                continue;

            SceneInfo* pSceneInfo = &pComponentSystemManager->m_pSceneInfoMap[i];

            GameObject* first = (GameObject*)pSceneInfo->m_GameObjects.GetHead();
            if( first && ( first->GetSceneID() == sceneid || savingallscenes ) )
            {
                for( CPPListNode* pNode = first; pNode; pNode = pNode->GetNext() )
                {
                    GameObject* pGameObject = (GameObject*)pNode;
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