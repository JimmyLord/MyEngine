//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../Libraries/imgui/imgui_internal.h"

extern IMGUI_API ImGuiContext* GImGui;

namespace ImGuiExt
{

// Idea taken from: https://github.com/ocornut/imgui/issues/1351
bool WasItemActiveLastFrame()
{
    ImGuiContext& g = *GImGui;

    if( g.ActiveIdPreviousFrame )
    {
        return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
    }
    return false;
}

ImGuiID GetActiveItemId()
{
    ImGuiContext& g = *GImGui;
    
    return g.ActiveId;
}

ImGuiID GetLastItemId()
{
    ImGuiContext& g = *GImGui;

    ImGuiWindow* window = g.CurrentWindow;
    if( window )
        return window->DC.LastItemId;

    return 0;
}

}
