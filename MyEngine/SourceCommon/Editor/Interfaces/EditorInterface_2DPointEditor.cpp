//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

EditorInterface_2DPointEditor::EditorInterface_2DPointEditor()
{
    m_pCollisionObject = 0;

    m_pPoint = 0;

    m_IndexOfPointBeingDragged = -1;
    m_NewMousePress = false;
}

EditorInterface_2DPointEditor::~EditorInterface_2DPointEditor()
{
    SAFE_DELETE( m_pPoint );
}

void EditorInterface_2DPointEditor::OnActivated()
{
    MaterialDefinition* pMaterial = g_pEngineCore->m_pEditorState->m_pTransformGizmo->m_pMaterial_Translate1Axis[1];

    // create a gameobject for the points that we'll draw.
    if( m_pPoint == 0 )
    {
        GameObject* pGameObject;
        ComponentMesh* pComponentMesh;

        pGameObject = g_pComponentSystemManager->CreateGameObject( false, EngineCore::ENGINE_SCENE_ID ); // not managed.
        pGameObject->SetName( "2D point editor - point" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, EngineCore::ENGINE_SCENE_ID );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( pMaterial, 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->Create2DCircle( 0.25f, 20 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
        }

        m_pPoint = pGameObject;
    }
}

void EditorInterface_2DPointEditor::OnDeactivated()
{
    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( false );
}

void EditorInterface_2DPointEditor::OnDrawFrame(unsigned int canvasid)
{
    // EditorInterface class will draw the main editor view
    EditorInterface::OnDrawFrame( canvasid );

    MyAssert( m_pCollisionObject != 0 );
    if( m_pCollisionObject == 0 )
        return;

    MyAssert( m_pPoint != 0 );
    if( m_pPoint == 0 )
        return;

    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( true );

    // Draw a circle at each vertex position. // lines are drawn by m_pCollisionObject's render callback
    for( unsigned int i=0; i<m_pCollisionObject->m_Vertices.size(); i++ )
    {
        b2Vec2 pos2d = m_pCollisionObject->m_Vertices[i];
        Vector3 pos3d( pos2d.x, pos2d.y, 0 );

        ComponentTransform* pParentTransformComponent = m_pCollisionObject->m_pGameObject->GetTransform();
        MyMatrix* pParentMatrix = pParentTransformComponent->GetWorldTransform();
        Vector3 worldpos = pParentMatrix->GetTranslation() + pos3d;

        m_pPoint->m_pComponentTransform->SetLocalPosition( worldpos );

        ComponentCamera* pCamera = g_pEngineCore->m_pEditorState->GetEditorCamera();
        MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

        float distance = (pCamera->m_pComponentTransform->GetLocalPosition() - pos3d).Length();
        m_pPoint->m_pComponentTransform->SetLocalScale( Vector3( distance / 15.0f ) );

        // TODO: change the material color if this is the selected dot.

        g_pComponentSystemManager->DrawSingleObject( pEditorMatViewProj, m_pPoint, 0 );
    }

    // Draw Box2D debug data
    if( g_pEngineCore->m_Debug_DrawPhysicsDebugShapes && g_GLCanvasIDActive == 1 )
    {
        for( int i=0; i<g_pComponentSystemManager->MAX_SCENES_LOADED; i++ )
        {
            if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse && g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld )
                g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->m_pWorld->DrawDebugData();
        }
    }
}

bool EditorInterface_2DPointEditor::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_ESC )
    {
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

    // TODO: store last used vertex and handle delete key
    if( keyaction == GCBA_Up && keycode == MYKEYCODE_DELETE )
    {
        if( m_IndexOfPointBeingDragged != -1 && m_pCollisionObject->m_Vertices.size() > 2 )
        {
            m_pCollisionObject->m_Vertices.erase( m_pCollisionObject->m_Vertices.begin() + m_IndexOfPointBeingDragged );
            m_IndexOfPointBeingDragged = -1;
        }
    }

    EditorInterface::SetModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        if( id == 0 ) // left mouse button
        {
            if( mouseaction == GCBA_Down )
            {
                m_NewMousePress = true;

                // find the object we clicked on.
                unsigned int pixelid = GetIDAtPixel( (unsigned int)x, (unsigned int)y, true );

                m_IndexOfPointBeingDragged = pixelid - 1;

                //LOGInfo( LOGTag, "Grabbed point %d\n", m_IndexOfPointBeingDragged );
            }

            if( mouseaction == GCBA_Held && id == 0 )
            {
                bool createnewvertex = false;
                if( m_NewMousePress && pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
                {
                    createnewvertex = true;
                }

                m_NewMousePress = false;

                if( m_IndexOfPointBeingDragged != -1 )
                {
                    //LOGInfo( LOGTag, "Released point %d\n", m_IndexOfPointBeingDragged );

                    // create a plane based on the axis we want.
                    Vector3 normal = Vector3(0,0,1);
                    Vector3 axisvector = Vector3(1,0,0);

                    // create a plane on Z = 0 // TODO: if the Box2D world is on any other plane, fix this.
                    Plane plane;
                    plane.Set( normal, Vector3(0,0,0) );

                    // Get the mouse click ray... current and last frame.
                    Vector3 currentraystart, currentrayend;
                    g_pEngineCore->GetMouseRay( pEditorState->m_CurrentMousePosition, &currentraystart, &currentrayend );

                    Vector3 lastraystart, lastrayend;
                    g_pEngineCore->GetMouseRay( pEditorState->m_LastMousePosition, &lastraystart, &lastrayend );

                    //LOGInfo( LOGTag, "current->(%0.0f,%0.0f) (%0.2f,%0.2f,%0.2f) (%0.2f,%0.2f,%0.2f)\n",
                    //        pEditorState->m_CurrentMousePosition.x,
                    //        pEditorState->m_CurrentMousePosition.y,
                    //        currentraystart.x,
                    //        currentraystart.y,
                    //        currentraystart.z,
                    //        currentrayend.x,
                    //        currentrayend.y,
                    //        currentrayend.z
                    //    );

                    // find the intersection point of the plane.
                    Vector3 currentresult;
                    Vector3 lastresult;
                    if( plane.IntersectRay( currentraystart, currentrayend, &currentresult ) &&
                        plane.IntersectRay( lastraystart, lastrayend, &lastresult ) )
                    {
                        //LOGInfo( LOGTag, "currentresult( %f, %f, %f );", currentresult.x, currentresult.y, currentresult.z );
                        //LOGInfo( LOGTag, "lastresult( %f, %f, %f );", lastresult.x, lastresult.y, lastresult.z );
                        //LOGInfo( LOGTag, "axisvector( %f, %f, %f );\n", axisvector.x, axisvector.y, axisvector.z );

                        ComponentTransform* pParentTransformComponent = m_pCollisionObject->m_pGameObject->GetTransform();
                        MyMatrix* pParentMatrix = pParentTransformComponent->GetLocalTransform();

                        b2Vec2 newpos;
                        newpos.x = currentresult.x - pParentMatrix->GetTranslation().x;
                        newpos.y = currentresult.y - pParentMatrix->GetTranslation().y;

                        if( createnewvertex )
                        {
                            std::vector<b2Vec2>::iterator it = m_pCollisionObject->m_Vertices.begin();
                            m_pCollisionObject->m_Vertices.insert( it + m_IndexOfPointBeingDragged, newpos );
                        }
                        else
                        {
                            m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged].Set( newpos.x, newpos.y );
                        }
                    }
                }

                //m_IndexOfPointBeingDragged = -1;
            }
        }
    }

    // handle camera movement, with both mouse and keyboard.
    EditorInterface::HandleInputForEditorCamera( keyaction, keycode, mouseaction, id, x, y, pressure );

    // clear mouse button states.
    EditorInterface::ClearModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    return false;
}

void EditorInterface_2DPointEditor::Set2DCollisionObjectToEdit(Component2DCollisionObject* pCollisionObject)
{
    m_pCollisionObject = pCollisionObject;
}

void EditorInterface_2DPointEditor::RenderObjectIDsToFBO()
{
    //EditorInterface::RenderObjectIDsToFBO();

    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    if( pEditorState->m_pMousePickerFBO->m_FullyLoaded == false )
        return;

    // bind our FBO so we can render the scene to it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    //pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( true );

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, pEditorState->m_pMousePickerFBO->m_Width, pEditorState->m_pMousePickerFBO->m_Height );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //// draw all editor camera components.
    //ComponentCamera* pCamera = 0;
    //for( unsigned int i=0; i<pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    //{
    //    pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->m_Components[i] );
    //    if( pCamera )
    //    {
    //        g_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, g_pEngineCore->m_pShader_TintColor );
    //        glClear( GL_DEPTH_BUFFER_BIT );
    //    }
    //}

    // Draw a circle at each vertex position.
    {
        ShaderGroup* pShaderOverride = g_pEngineCore->m_pShader_TintColor;
        Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    
        if( pShader->ActivateAndProgramShader() )
        {
            ComponentCamera* pCamera = g_pEngineCore->m_pEditorState->GetEditorCamera();
            MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

            for( unsigned int i=0; i<m_pCollisionObject->m_Vertices.size(); i++ )
            {
                b2Vec2 pos2d = m_pCollisionObject->m_Vertices[i];
                Vector3 pos3d( pos2d.x, pos2d.y, 0 );

                ComponentTransform* pParentTransformComponent = m_pCollisionObject->m_pGameObject->GetTransform();
                MyMatrix* pParentMatrix = pParentTransformComponent->GetLocalTransform();
                Vector3 worldpos = pParentMatrix->GetTranslation() + pos3d;

                m_pPoint->m_pComponentTransform->SetLocalPosition( worldpos );

                ColorByte tint( 0, 0, 0, 0 );
                    
                unsigned int id = (i+1) * 641; // 1, 641, 6700417, 4294967297, 

                if( 1 )                 tint.r = id%256;
                if( id > 256 )          tint.g = (id>>8)%256;
                if( id > 256*256 )      tint.b = (id>>16)%256;
                if( id > 256*256*256 )  tint.a = (id>>24)%256;

                pShader->ProgramTint( tint );

                g_pComponentSystemManager->DrawSingleObject( pEditorMatViewProj, m_pPoint, pShaderOverride );
            }
        }
    }

    pEditorState->m_pMousePickerFBO->Unbind( true );

    pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( false );
}