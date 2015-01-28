//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameEditorCommands_H__
#define __GameEditorCommands_H__

class EditorCommand_MoveObjects : public EditorCommand
{
protected:
    Vector3 m_DistanceMoved;
    std::vector<GameObject*> m_ObjectsMoved;

public:
    EditorCommand_MoveObjects(Vector3 distancemoved, const std::vector<GameObject*> &selectedobjects);
    virtual ~EditorCommand_MoveObjects();

    virtual void Do();
    virtual void Undo();
};

#endif // __CommandStack_H__
