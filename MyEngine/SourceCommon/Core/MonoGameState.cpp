//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

#include "MonoGameState.h"

MonoGameState::MonoGameState(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    // Create the core mono JIT domain.
    mono_set_dirs( "./mono/lib", "" );
    m_pCoreDomain = mono_jit_init( "MyEngine" );

    m_pActiveDomain = nullptr;
}

MonoGameState::~MonoGameState()
{
    mono_jit_cleanup( m_pCoreDomain );
}

void MonoGameState::Rebuild()
{
    // If the domain has been created and an assembly loaded, then destroy that domain and rebuild.
    if( m_pActiveDomain )
    {
        mono_domain_set( m_pCoreDomain, true );
        mono_domain_unload( m_pActiveDomain );
    }

    // Create a domain for the game assembly that we will load.
    m_pActiveDomain = mono_domain_create_appdomain( "TestDomain", nullptr );
    // Set the new domain as active.
    mono_domain_set( m_pActiveDomain, true );

    // Load the assembly and grab a pointer to the image from the assembly.
    MonoAssembly* pMonoAssembly = mono_domain_assembly_open( m_pActiveDomain, "Data/Mono/Game.dll" );
    m_pMonoImage = mono_assembly_get_image( pMonoAssembly );
}
