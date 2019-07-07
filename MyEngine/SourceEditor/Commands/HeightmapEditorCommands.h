//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __HeightmapEditorCommands_H__
#define __HeightmapEditorCommands_H__

#include "ComponentSystem/EngineComponents/ComponentHeightmap.h"

class EditorCommand_Heightmap_Raise;

//====================================================================================================
// EditorCommand_Heightmap_Raise
//====================================================================================================

class EditorCommand_Heightmap_Raise : public EditorCommand
{
protected:
    ComponentHeightmap* m_pHeightmap;
    Vector3 m_Position;
    float m_BrushSoftness;
    float m_RaiseAmount;
    float m_BrushRadius;

public:
    EditorCommand_Heightmap_Raise(ComponentHeightmap* pHeightmap, Vector3 position, float amount, float radius, float softness);
    virtual ~EditorCommand_Heightmap_Raise();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

#endif // __HeightmapEditorCommands_H__
