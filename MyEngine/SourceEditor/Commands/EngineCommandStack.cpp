//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EngineCommandStack.h"
#include "Core/EngineCore.h"

EngineCommandStack::EngineCommandStack()
{
}

EngineCommandStack::~EngineCommandStack()
{
}

void EngineCommandStack::Undo(unsigned int levels)
{
    if( g_pEngineCore->IsInEditorMode() )
        CommandStack::Undo( levels );
}

void EngineCommandStack::Redo(unsigned int levels)
{
    if( g_pEngineCore->IsInEditorMode() )
        CommandStack::Redo( levels );
}

void EngineCommandStack::Do(EditorCommand* pCommand, bool linktoprevious, bool autolinkifsameframeasprevious)
{
    if( g_pEngineCore->IsInEditorMode() )
    {
        // if editor mode, add to undo stack then do command.
        CommandStack::Do( pCommand, linktoprevious, autolinkifsameframeasprevious );
    }
    else
    {
        // if game mode, do the command and delete it.
        pCommand->Do();
        delete pCommand;
    }
}

void EngineCommandStack::Add(EditorCommand* pCommand, bool linktoprevious, bool autolinkifsameframeasprevious)
{
    if( g_pEngineCore->IsInEditorMode() )
        return CommandStack::Add( pCommand, linktoprevious, autolinkifsameframeasprevious );

    delete pCommand;
}
