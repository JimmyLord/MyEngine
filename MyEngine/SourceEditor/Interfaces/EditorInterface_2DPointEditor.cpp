//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
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

    m_PositionMouseWentDown.SetZero();

    m_IndexOfPointBeingDragged = -1;
    m_NewMousePress = false;
    m_AddedVertexWhenMouseWasDragged = false;

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        m_pMaterials[i] = 0;
    }
}

EditorInterface_2DPointEditor::~EditorInterface_2DPointEditor()
{
    SAFE_DELETE( m_pPoint );

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        SAFE_RELEASE( m_pMaterials[i] );
    }
}

void EditorInterface_2DPointEditor::Initialize()
{
    if( m_pMaterials[Mat_Lines] == 0 )
        m_pMaterials[Mat_Lines] = MyNew MaterialDefinition( g_pEngineCore->GetShader_TintColor(), ColorByte(255,0,0,255) );
    if( m_pMaterials[Mat_Points] == 0 )
        m_pMaterials[Mat_Points] = MyNew MaterialDefinition( g_pEngineCore->GetShader_TintColor(), ColorByte(255,255,0,255) );
    if( m_pMaterials[Mat_SelectedPoint] == 0 )
        m_pMaterials[Mat_SelectedPoint] = MyNew MaterialDefinition( g_pEngineCore->GetShader_TintColor(), ColorByte(255,255,255,255) );
}

void EditorInterface_2DPointEditor::OnActivated()
{
    // create a gameobject for the points that we'll draw.
    if( m_pPoint == 0 )
    {
        GameObject* pGameObject;
        ComponentMesh* pComponentMesh;

        pGameObject = g_pComponentSystemManager->CreateGameObject( false, SCENEID_EngineObjects ); // not managed.
        pGameObject->SetName( "2D point editor - point" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, SCENEID_EngineObjects );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterials[Mat_Points], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->Create2DCircle( 0.25f, 20 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;

            // remove this from the main list of renderable components.
            pComponentMesh->UnregisterCallbacks();
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

        m_pPoint->GetTransform()->SetLocalPosition( worldpos );

        ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
        MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

        float distance = (pCamera->m_pComponentTransform->GetLocalPosition() - worldpos).Length();
        m_pPoint->GetTransform()->SetLocalScale( Vector3( distance / 15.0f ) );

        // change the material color if this is the selected dot.
        if( i == (unsigned int)m_IndexOfPointBeingDragged )
            m_pPoint->SetMaterial( m_pMaterials[Mat_SelectedPoint] );

        g_pComponentSystemManager->DrawSingleObject( pEditorMatViewProj, m_pPoint, 0 );

        if( i == (unsigned int)m_IndexOfPointBeingDragged )
            m_pPoint->SetMaterial( m_pMaterials[Mat_Points] );
    }

    // Draw Box2D debug data
    if( g_pEngineCore->GetEditorPrefs()->Get_Debug_DrawPhysicsDebugShapes() && g_GLCanvasIDActive == 1 )
    {
        for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse && g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld )
                g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->m_pWorld->DrawDebugData();
        }
    }
}

void EditorInterface_2DPointEditor::CancelCurrentOperation()
{
    if( m_IndexOfPointBeingDragged != -1 )
    {
        if( m_AddedVertexWhenMouseWasDragged )
        {
            g_pGameCore->GetCommandStack()->Undo( 1 );
        }
        else
        {
            m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged] = m_PositionMouseWentDown;
        }
        m_IndexOfPointBeingDragged = -1;
    }
}

bool EditorInterface_2DPointEditor::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    bool inputhandled = false;

    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_ESC )
    {
        if( m_IndexOfPointBeingDragged != -1 )
            CancelCurrentOperation();
        else
            g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

    // TODO: store last used vertex and handle delete key
    if( keyaction == GCBA_Up && keycode == MYKEYCODE_DELETE )
    {
        if( m_IndexOfPointBeingDragged != -1 && m_pCollisionObject->m_Vertices.size() > 2 )
        {
            // if the mouse is still down, undo will use the position of the vertex when the mouse went down
            //     otherwise it will use the saved position of the vertex
            b2Vec2 position = m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged];
            if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
                position = m_PositionMouseWentDown;

            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_Delete2DPoint( m_pCollisionObject, m_IndexOfPointBeingDragged, position ) );
            m_IndexOfPointBeingDragged = -1;
        }
    }

    EditorInterface::SetModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        if( mouseaction == GCBA_Down )
        {
            if( id == 1 ) // right mouse button to cancel current operation
            {
                LOGDebug( "2DPointEditor", "Cancelled operation on point %d\n", m_IndexOfPointBeingDragged );
                CancelCurrentOperation();
                inputhandled = true;
            }
        }

        if( mouseaction == GCBA_Down && id == 0 ) // left mouse button down
        {
            m_NewMousePress = true;

            // find the object we clicked on.
            unsigned int pixelid = GetIDAtPixel( (unsigned int)x, (unsigned int)y, true, true );

            m_IndexOfPointBeingDragged = pixelid - 1;
            m_AddedVertexWhenMouseWasDragged = false;

            // reset mouse movement, so we can undo to this state after mouse goes up.
            if( m_IndexOfPointBeingDragged != -1 )
            {
                m_PositionMouseWentDown = m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged];
            }

            LOGDebug( "2DPointEditor", "Grabbed point %d\n", m_IndexOfPointBeingDragged );
        }

        if( mouseaction == GCBA_Up && id == 0 ) // left mouse button up
        {
            if( m_IndexOfPointBeingDragged != -1 )
            {
                LOGDebug( "2DPointEditor", "Released point %d\n", m_IndexOfPointBeingDragged );

                // add command to stack for undo/redo.                    
                b2Vec2 distmoved = m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged] - m_PositionMouseWentDown;
                if( distmoved.LengthSquared() != 0 )
                {
                    g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_Move2DPoint( distmoved, m_pCollisionObject, m_IndexOfPointBeingDragged ), m_AddedVertexWhenMouseWasDragged );
                }

                m_IndexOfPointBeingDragged = -1;
            }
        }

        if( mouseaction == GCBA_Held && id == 0 ) //id & 1 << 0 ) // left mouse button moved
        {
            bool createnewvertex = false;
            if( m_NewMousePress && pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
            {
                createnewvertex = true;
            }

            m_NewMousePress = false;

            if( m_IndexOfPointBeingDragged != -1 )
            {
                //LOGDebug( "2DPointEditor", "Moved point %d\n", m_IndexOfPointBeingDragged );

                // create a plane based on the axis we want.
                Vector3 normal = Vector3(0,0,1);
                Vector3 axisvector = Vector3(1,0,0);

                ComponentTransform* pParentTransformComponent = m_pCollisionObject->m_pGameObject->GetTransform();
                MyMatrix* pParentMatrix = pParentTransformComponent->GetWorldTransform();
                Vector3 worldpos = pParentMatrix->GetTranslation();

                // create a plane on Z = worldpos.z // TODO: if the Box2D world is on a rotated plane, fix this.
                Plane plane;
                plane.Set( normal, Vector3( 0, 0, worldpos.z ) );

                // Get the mouse click ray... current and last frame.
                Vector3 currentraystart, currentrayend;
                g_pEngineCore->GetMouseRay( pEditorState->m_CurrentMousePosition, &currentraystart, &currentrayend );

                Vector3 lastraystart, lastrayend;
                g_pEngineCore->GetMouseRay( pEditorState->m_LastMousePosition, &lastraystart, &lastrayend );

                //LOGDebug( "2DPointEditor", "current->(%0.0f,%0.0f) (%0.2f,%0.2f,%0.2f) (%0.2f,%0.2f,%0.2f)\n",
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
                    //LOGDebug( "2DPointEditor", "currentresult( %f, %f, %f );", currentresult.x, currentresult.y, currentresult.z );
                    //LOGDebug( "2DPointEditor", "lastresult( %f, %f, %f );", lastresult.x, lastresult.y, lastresult.z );
                    //LOGDebug( "2DPointEditor", "axisvector( %f, %f, %f );\n", axisvector.x, axisvector.y, axisvector.z );

                    ComponentTransform* pParentTransformComponent = m_pCollisionObject->m_pGameObject->GetTransform();
                    MyMatrix* pParentMatrix = pParentTransformComponent->GetLocalTransform();

                    b2Vec2 newpos;
                    newpos.x = currentresult.x - pParentMatrix->GetTranslation().x;
                    newpos.y = currentresult.y - pParentMatrix->GetTranslation().y;

                    if( g_pEngineCore->GetEditorPrefs()->Get_Grid_SnapEnabled() )
                    {
                        // snap point to grid.
                        newpos.x = MyRoundToMultipleOf( newpos.x, g_pEngineCore->GetEditorPrefs()->GetGridSettings()->stepsize.x );
                        newpos.y = MyRoundToMultipleOf( newpos.y, g_pEngineCore->GetEditorPrefs()->GetGridSettings()->stepsize.y );
                    }

                    if( createnewvertex )
                    {
                        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_Insert2DPoint( m_pCollisionObject, m_IndexOfPointBeingDragged ) );
                        m_AddedVertexWhenMouseWasDragged = true;
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

    // handle camera movement, with both mouse and keyboard.
    if( inputhandled == false )
    {
        EditorInterface::HandleInputForEditorCamera( keyaction, keycode, mouseaction, id, x, y, pressure );
    }

    return false;
}

void EditorInterface_2DPointEditor::Set2DCollisionObjectToEdit(Component2DCollisionObject* pCollisionObject)
{
    m_pCollisionObject = pCollisionObject;

    if( m_pCollisionObject->m_Vertices.size() == 0 )
    {
        m_pCollisionObject->m_Vertices.push_back( b2Vec2(0,0) );
    }
}

void EditorInterface_2DPointEditor::RenderObjectIDsToFBO()
{
    // Draw a circle for each 2D point, so the mouse can select them.

    // Don't call the base class, which would draw all scene objects.
    //EditorInterface::RenderObjectIDsToFBO();

    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return;

    // Bind our FBO so we can render stuff to it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, pEditorState->m_pMousePickerFBO->GetWidth(), pEditorState->m_pMousePickerFBO->GetHeight() );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Draw a circle at each vertex position.
    {
        ShaderGroup* pShaderOverride = g_pEngineCore->GetShader_TintColor();
        Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    
        if( pShader->ActivateAndProgramShader() )
        {
            ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
            MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

            for( unsigned int i=0; i<m_pCollisionObject->m_Vertices.size(); i++ )
            {
                b2Vec2 pos2d = m_pCollisionObject->m_Vertices[i];
                Vector3 pos3d( pos2d.x, pos2d.y, 0 );

                ComponentTransform* pParentTransformComponent = m_pCollisionObject->m_pGameObject->GetTransform();
                MyMatrix* pParentMatrix = pParentTransformComponent->GetLocalTransform();
                Vector3 worldpos = pParentMatrix->GetTranslation() + pos3d;

                m_pPoint->GetTransform()->SetLocalPosition( worldpos );

                ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
                MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

                float distance = (pCamera->m_pComponentTransform->GetLocalPosition() - pos3d).Length();
                m_pPoint->GetTransform()->SetLocalScale( Vector3( distance / 15.0f ) );

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
}

MaterialDefinition* EditorInterface_2DPointEditor::GetMaterial(MaterialTypes type)
{
    MyAssert( type >= 0 && type < Mat_NumMaterials );

    return m_pMaterials[type];
}
