//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#include "BulletDebugDraw.h"

BulletDebugDraw::BulletDebugDraw(MaterialDefinition* debugdrawmaterial, MyMatrix* matviewproj)
{
    m_pMatViewProj = matviewproj;

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

void BulletDebugDraw::Draw(const Vector3* vertices, uint32 vertexCount, ColorByte color, int primitivetype, float pointorlinesize)
{
    // Set the material to the correct color and draw the shape.
    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass( 0, 0 );
    if( pShader->ActivateAndProgramShader() == false )
        return;

    m_pMaterial->SetColorDiffuse( color );

    // Setup our position attribute, pass in the array of verts, not using a VBO.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    pShader->InitializeAttributeArray( pShader->m_aHandle_Position, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)vertices );

    // Setup uniforms, mainly viewproj and tint.
    pShader->ProgramBaseUniforms( m_pMatViewProj, 0, 0, m_pMaterial->m_ColorDiffuse, m_pMaterial->m_ColorSpecular, m_pMaterial->m_Shininess );

    glLineWidth( pointorlinesize );
#ifndef MYFW_OPENGLES2
    glPointSize( pointorlinesize );
#endif

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );

    MyDrawArrays( primitivetype, 0, vertexCount, false );

    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );

    glDisable( GL_BLEND );
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    Vector3 points[2];
    points[0].Set( from.getX(), from.getY(), from.getZ() );
    points[1].Set( to.getX(), to.getY(), to.getZ() );
    
    ColorByte colorbyte( (unsigned char)color.getX()*255, (unsigned char)color.getY()*255, (unsigned char)color.getZ()*255, 255 );
    Draw( points, 2, colorbyte, GL_LINES, 2 );
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
