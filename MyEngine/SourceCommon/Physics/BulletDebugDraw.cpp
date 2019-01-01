//
// Copyright (c) 2017-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/Renderer_Enums.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/Renderer_Base.h"

#include "BulletDebugDraw.h"

// TODO: Fix GL Includes.
#include <gl/GL.h>
#include "../../../../Framework/MyFramework/SourceWindows/GLExtensions.h"
#include "../../../../Framework/MyFramework/SourceCommon/Shaders/GLHelpers.h"

BulletDebugDraw::BulletDebugDraw(MaterialDefinition* debugdrawmaterial, MyMatrix* pMatProj, MyMatrix* pMatView)
{
    m_pMatProj = pMatProj;
    m_pMatView = pMatView;

    m_pMaterial = debugdrawmaterial;
    if( m_pMaterial )
    {
        m_pMaterial->AddRef();
    }
}

BulletDebugDraw::~BulletDebugDraw()
{
    SAFE_RELEASE( m_pMaterial );
}

void BulletDebugDraw::Draw(const Vector3* vertices, uint32 vertexCount, ColorByte color, MyRE::PrimitiveTypes primitiveType, float pointOrLineSize)
{
    // Set the material to the correct color and draw the shape.
    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass( 0, 0 );
    if( pShader->Activate() == false )
        return;

    m_pMaterial->SetColorDiffuse( color );

    // Setup our position attribute, pass in the array of verts, not using a VBO.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    pShader->InitializeAttributeArray( pShader->m_aHandle_Position, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)vertices );

    // Setup uniforms, mainly viewproj and tint.
    pShader->ProgramMaterialProperties( 0, m_pMaterial->m_ColorDiffuse, m_pMaterial->m_ColorSpecular, m_pMaterial->m_Shininess );
    pShader->ProgramTransforms( m_pMatProj, m_pMatView, 0 );

    glLineWidth( pointOrLineSize );
#ifndef MYFW_OPENGLES2
    glPointSize( pointOrLineSize );
#endif

    g_pRenderer->SetBlendEnabled( true );
    g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );

    glDisable( GL_CULL_FACE );
    g_pRenderer->SetDepthTestEnabled( false );

    g_pRenderer->DrawArrays( primitiveType, 0, vertexCount, false );

    glEnable( GL_CULL_FACE );
    g_pRenderer->SetDepthTestEnabled( true );

    // Always disable blending.
    g_pRenderer->SetBlendEnabled( false );
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    Vector3 points[2];
    points[0].Set( from.getX(), from.getY(), from.getZ() );
    points[1].Set( to.getX(), to.getY(), to.getZ() );
    
    ColorByte colorbyte( (unsigned char)color.getX()*255, (unsigned char)color.getY()*255, (unsigned char)color.getZ()*255, 255 );
    Draw( points, 2, colorbyte, MyRE::PrimitiveType_Lines, 2 );
}

void BulletDebugDraw::drawLine(const btVector3& from,const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
    btIDebugDraw::drawLine( from, to, fromColor, toColor );
}

void BulletDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
}

void BulletDebugDraw::reportErrorWarning(const char* warningString)
{
}

void BulletDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
}

void BulletDebugDraw::setDebugMode(int debugMode)
{
    m_DebugDrawMode = (DebugDrawModes)debugMode;
}

int BulletDebugDraw::getDebugMode() const
{
    return m_DebugDrawMode;
}
