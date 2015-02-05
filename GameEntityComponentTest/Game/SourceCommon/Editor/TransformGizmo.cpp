//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"
#include "TransformGizmo.h"

TransformGizmo::TransformGizmo()
{
    m_SelectedPart = -1;

    for( int i=0; i<3; i++ )
        m_pTransformGizmos[i] = 0;
}

TransformGizmo::~TransformGizmo()
{
    for( int i=0; i<3; i++ )
    {
        SAFE_DELETE( m_pTransformGizmos[i] );
    }
}

void TransformGizmo::Tick(double TimePassed, EditorState* pEditorState)
{
    // Update transform gizmos
    for( int i=0; i<3; i++ )
    {
        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTransformGizmos[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

        ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );
        assert( pMesh );
        if( pMesh )
        {
            if( i == 0 )
                pMesh->m_pMesh->m_Tint.Set( 255, 100, 100, 255 );
            if( i == 1 )
                pMesh->m_pMesh->m_Tint.Set( 100, 255, 100, 255 );
            if( i == 2 )
                pMesh->m_pMesh->m_Tint.Set( 100, 100, 255, 255 );

            if( i == m_SelectedPart )
            {
                pMesh->m_pMesh->m_Tint.Set( 255,255,255,255 );
            }
        }

        Vector3 ObjectPosition;
        MyMatrix ObjectTransform;

        if( pEditorState->m_pSelectedObjects.size() == 1 )
        {
            pRenderable->m_Visible = true;
            ObjectPosition = pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetPosition();
            ObjectTransform = *pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetLocalTransform();
        }
        else if( pEditorState->m_pSelectedObjects.size() > 1 )
        {
            pRenderable->m_Visible = true;

            // find the center point between all selected objects.
            ObjectPosition.Set( 0, 0, 0 );
            for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
            {
                ObjectPosition += pEditorState->m_pSelectedObjects[i]->m_pComponentTransform->GetPosition();
            }
            ObjectPosition /= pEditorState->m_pSelectedObjects.size();

            ObjectTransform.SetIdentity();
        }
        else
        {
            pRenderable->m_Visible = false;
        }

        if( pRenderable->m_Visible )
        {
            // move the gizmo to the object position.
            m_pTransformGizmos[i]->m_pComponentTransform->SetPosition( ObjectPosition );

            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();
            if( i == 0 )
                matrot.Rotate( 90, 0, 0, 1 );
            if( i == 1 )
                matrot.Rotate( 0, 1, 0, 0 );
            if( i == 2 )
                matrot.Rotate( -90, 1, 0, 0 );

            MyMatrix matrotobj;
            matrotobj.SetIdentity();
            matrotobj.CreateSRT( Vector3(1,1,1), ObjectTransform.GetEulerAngles(), Vector3(0,0,0) );

            matrot = matrotobj * matrot;

            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI;

            m_pTransformGizmos[i]->m_pComponentTransform->SetRotation( rot );

            float distance = (pEditorState->m_pEditorCamera->m_pComponentTransform->GetPosition() - ObjectPosition).Length();
            m_pTransformGizmos[i]->m_pComponentTransform->SetScale( Vector3( distance / 15.0f ) );
        }
    }
}

bool TransformGizmo::HandleInput(GameEntityComponentTest* pGame, int keydown, int keycode, int action, int id, float x, float y, float pressure)
{
    // find the object we're hovering on
    GameObject* pObject = pGame->GetObjectAtPixel( x, y );

    m_SelectedPart = -1;

    if( pObject == m_pTransformGizmos[0] )
        m_SelectedPart = 0;
    if( pObject == m_pTransformGizmos[1] )
        m_SelectedPart = 1;
    if( pObject == m_pTransformGizmos[2] )
        m_SelectedPart = 2;

    return false;
}

void TransformGizmo::ScaleGizmosForMousePickRendering(bool doscale)
{
    float scaleamount = 3;

    if( doscale == false )
        scaleamount = 1/scaleamount;

    for( int i=0; i<3; i++ )
    {
        Vector3 currentscale = m_pTransformGizmos[i]->m_pComponentTransform->GetLocalScale();
        Vector3 newscale( currentscale.x * scaleamount, currentscale.y, currentscale.z * scaleamount );
        m_pTransformGizmos[i]->m_pComponentTransform->SetScale( newscale );
    }
}
