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

#include "emscripten.h"
#include <sys/time.h>
#include <SDL/SDL.h>

#include <stdlib.h>
#include "esUtil.h"

void MainLoop();
void PollSDLEvents();

static double g_lasttime;

//TODO: look into this javascript warning
// Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!

int main(int argc, char *argv[])
{
    SDL_Init( SDL_INIT_VIDEO );
    SDL_Surface* screen = SDL_SetVideoMode( 768, 432, 32, SDL_OPENGL );

    g_pGameCore = MyNew GameEntityComponentTest;

    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, 768, 432 );//SCREEN_WIDTH, SCREEN_HEIGHT );
    g_pGameCore->OneTimeInit();

    g_lasttime = MyTime_GetSystemTime();

    emscripten_set_main_loop( MainLoop, 60, false );
}

void MainLoop()
{
    // Main loop
    double currtime = MyTime_GetSystemTime();
    double timepassed = currtime - g_lasttime;
    g_lasttime = currtime;
        
    PollSDLEvents();

    g_UnpausedTime += g_pGameCore->Tick( timepassed );
    g_pGameCore->OnDrawFrame();
}

void PollSDLEvents()
{
    //LOGInfo( LOGTag, "PollSDLEvents\n" );

    SDL_Event event;
    SDL_MouseMotionEvent* mm = 0;
    SDL_MouseButtonEvent* mb = 0;
    SDL_KeyboardEvent* kb = 0;

    while( SDL_PollEvent( &event ) )
    {
        //LOGInfo( LOGTag, "Processing SDLEvent\n" );

        switch( event.type )
        {
        case SDL_KEYDOWN:
            kb = (SDL_KeyboardEvent*)&event;
            //printf( "key down: %d,%d  %d\n", kb->state, kb->repeat, kb->keysym.sym );
            if( kb->keysym.sym == 27 ) // ESCAPE
                g_pGameCore->OnButtons( GCBA_Down, GCBI_Back );
            if( kb->keysym.sym == 1104 ) // left arrow numpad
                g_pGameCore->OnButtons( GCBA_Down, GCBI_Left );
            if( kb->keysym.sym == 1106 ) // up arrow numpad
                g_pGameCore->OnButtons( GCBA_Down, GCBI_Up );
            if( kb->keysym.sym == 1103 ) // right arrow numpad
                g_pGameCore->OnButtons( GCBA_Down, GCBI_Right );
            if( kb->keysym.sym == 1105 ) // down arrow numpad
                g_pGameCore->OnButtons( GCBA_Down, GCBI_Down );
            if( kb->keysym.sym == 'z' )
                g_pGameCore->OnButtons( GCBA_Down, GCBI_ButtonA );
            if( kb->keysym.sym == 'x' )
                g_pGameCore->OnButtons( GCBA_Down, GCBI_ButtonB );
            if( kb->keysym.sym == 'c' )
                g_pGameCore->OnButtons( GCBA_Down, GCBI_ButtonC );
            if( kb->keysym.sym == 'v' )
                g_pGameCore->OnButtons( GCBA_Down, GCBI_ButtonD );

            else if( kb->keysym.sym >= 'a' && kb->keysym.sym <= 'z' &&
                     (kb->keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0 )
            {
                g_pGameCore->OnKeyDown( kb->keysym.sym-32, kb->keysym.sym-32 );
            }
            else
            {
                g_pGameCore->OnKeyDown( kb->keysym.sym, kb->keysym.sym );
            }
            break;

        case SDL_KEYUP:
            kb = (SDL_KeyboardEvent*)&event;
            //printf( "key up: %d,%d  %d\n", kb->state, kb->repeat, kb->keysym.sym );
            if( kb->keysym.sym == 27 ) // ESCAPE
                g_pGameCore->OnButtons( GCBA_Up, GCBI_Back );
            if( kb->keysym.sym == 1104 ) // left arrow numpad
                g_pGameCore->OnButtons( GCBA_Up, GCBI_Left );
            if( kb->keysym.sym == 1106 ) // up arrow numpad
                g_pGameCore->OnButtons( GCBA_Up, GCBI_Up );
            if( kb->keysym.sym == 1103 ) // right arrow numpad
                g_pGameCore->OnButtons( GCBA_Up, GCBI_Right );
            if( kb->keysym.sym == 1105 ) // down arrow numpad
                g_pGameCore->OnButtons( GCBA_Up, GCBI_Down );
            if( kb->keysym.sym == 'z' )
                g_pGameCore->OnButtons( GCBA_Up, GCBI_ButtonA );
            if( kb->keysym.sym == 'x' )
                g_pGameCore->OnButtons( GCBA_Up, GCBI_ButtonB );
            if( kb->keysym.sym == 'c' )
                g_pGameCore->OnButtons( GCBA_Up, GCBI_ButtonC );
            if( kb->keysym.sym == 'v' )
                g_pGameCore->OnButtons( GCBA_Up, GCBI_ButtonD );

            else if( kb->keysym.sym >= 'a' && kb->keysym.sym <= 'z' &&
                     (kb->keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0 )
            {
                g_pGameCore->OnKeyUp( kb->keysym.sym-32, kb->keysym.sym-32 );
            }
            else
            {
                g_pGameCore->OnKeyUp( kb->keysym.sym, kb->keysym.sym );
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            mb = (SDL_MouseButtonEvent*)&event;
            //printf( "button down: %d,%d  %d,%d\n", mb->button, mb->state, mb->x, mb->y );
            g_pGameCore->OnTouch( GCBA_Down, 0, mb->x, mb->y, 0, 0 );
            break;

        case SDL_MOUSEBUTTONUP:
            mb = (SDL_MouseButtonEvent*)&event;
            //printf( "button up: %d,%d  %d,%d\n", mb->button, mb->state, mb->x, mb->y );
            g_pGameCore->OnTouch( GCBA_Up, 0, mb->x, mb->y, 0, 0 );
            break;

        case SDL_MOUSEMOTION:
            mm = (SDL_MouseMotionEvent*)&event;
            //printf( "mouse move: %d  %d,%d\n", mm->state, mm->x, mm->y );
            g_pGameCore->OnTouch( GCBA_Held, 0, mm->x, mm->y, 0, 0 );
            break;
        }
    }
}
