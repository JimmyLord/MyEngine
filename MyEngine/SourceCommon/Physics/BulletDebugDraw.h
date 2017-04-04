//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __BulletDebugDraw_H__
#define __BulletDebugDraw_H__

class BulletDebugDraw : public btIDebugDraw
{
public:
    DebugDrawModes m_DebugDrawMode;

    MaterialDefinition* m_pMaterial;
    MyMatrix* m_pMatViewProj;

public:
    BulletDebugDraw(MaterialDefinition* debugdrawmaterial, MyMatrix* matviewproj);
    ~BulletDebugDraw();

    virtual void Draw(const Vector3* vertices, uint32 vertexCount, ColorByte color, int primitivetype, float pointorlinesize);

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
    virtual void drawLine(const btVector3& from,const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
    virtual void reportErrorWarning(const char* warningString);
    virtual void draw3dText(const btVector3& location, const char* textString);
    virtual void setDebugMode(int debugMode);
    virtual int getDebugMode() const;
};

#endif //__BulletDebugDraw_H__
