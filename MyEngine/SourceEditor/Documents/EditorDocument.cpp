//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorDocument.h"

bool EditorDocument::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    ImGuiIO& io = ImGui::GetIO();

    if( keyAction == GCBA_Down )
    {
        bool N  = !io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper; // No modifiers held
        bool C  =  io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper; // Ctrl
        bool A  = !io.KeyCtrl &&  io.KeyAlt && !io.KeyShift && !io.KeySuper; // Alt
        bool S  = !io.KeyCtrl && !io.KeyAlt &&  io.KeyShift && !io.KeySuper; // Shift
        bool CS =  io.KeyCtrl && !io.KeyAlt &&  io.KeyShift && !io.KeySuper; // Ctrl-Shift

        if( C  && keyCode == 'Z' )
        {
            if( m_pCommandStack->GetUndoStackSize() > 0 )
                m_pCommandStack->Undo( 1 );

            return true;
        }
        if( C  && keyCode == 'Y' )
        {
            if( m_pCommandStack->GetRedoStackSize() > 0 )
                m_pCommandStack->Redo( 1 );

            return true;
        }
        if( CS && keyCode == 'Z' )
        {
            if( m_pCommandStack->GetRedoStackSize() > 0 )
                m_pCommandStack->Redo( 1 );

            return true;
        }
        if( C  && keyCode == 'S' ) { m_SaveRequested = true; return true; }
    }

    return false;
}
