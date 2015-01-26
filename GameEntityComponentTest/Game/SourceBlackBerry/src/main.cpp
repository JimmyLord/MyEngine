//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"
#include "Core/GameEntityComponentTest.h"

#include "../../MyFramework/SourceBlackBerry/BBMain.h"

// ScoreLoop simple demo... good for testing lots of scores(but it works, so useless :))
const char* g_ScoreLoop_GameID = "b0c5ba61-fdf4-4744-a0d2-d3e34bc20cd5";
const char* g_ScoreLoop_GameSecret = "2PCCEsjTS+Wty7t6ZC0kJrOuB7dNbloP7zeSBErM8dYqJgowqWzn9A==";
const char* g_ScoreLoop_VersionNumber = "1.0";
const char* g_ScoreLoop_GameCurrencyCode = "XWX";
const char* g_ScoreLoop_Languages = "en";

bool BBFUNC_HandleKeyboardEvents(GameCoreButtonActions action, int keycode)
{
    if( action == GCBA_Down )
    {
        g_pGameCore->OnKeyDown( keycode, keycode );
    }
    if( action == GCBA_Up )
    {
        g_pGameCore->OnKeyUp( keycode, keycode );
    }

    return true;
}

int main(int argc, char *argv[])
{
    g_pGameCore = MyNew GameEntityComponentTest;

    g_BBMain_EGLSwapInterval = 2; // 30 fps cap

    bbmain( "zzzzzzzz-zzzz-zzzz-zzzz-zzzzzzzzzzzz" ); // Get a real BBM key
}
