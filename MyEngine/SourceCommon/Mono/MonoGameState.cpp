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
#include "Mono/metadata/mono-debug.h"

#include "MonoGameState.h"
#if MYFW_USING_MONO
#include "ComponentSystem/EngineComponents/ComponentMonoScript.h"
#endif //MYFW_USING_MONO
#include "Core/EngineCore.h"
#include "Mono/BaseComponents/MonoComponentMesh.h"
#include "Mono/BaseComponents/MonoComponentMeshPrimitive.h"
#include "Mono/BaseComponents/MonoComponentTransform.h"
#include "Mono/Core/MonoComponentSystemManager.h"
#include "Mono/Core/MonoEngineCore.h"
#include "Mono/Core/MonoGameObject.h"
#include "Mono/Framework/MonoFrameworkClasses.h"
#include "Mono/Framework/MonoMaterialDefinition.h"
#include "Mono/Framework/MonoMaterialManager.h"

#if MYFW_EDITOR
#if MYFW_WINDOWS
#pragma warning( push )
#pragma warning( disable:4267 )
#include "../../Libraries/dirent/dirent.h"
#pragma warning( pop )
#else
#include <dirent.h>
#endif
#endif

MonoDomain* MonoGameState::g_pActiveDomain = nullptr;
MonoImage* MonoGameState::g_pMonoImage = nullptr;

MonoGameState::MonoGameState(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    // Setup debugger.

    // From mono\mini\debugger-agent.c
    //   Usage: mono --debugger-agent=[<option>=<value>,...] ...
    //   Available options:
    //     transport=<transport>        Transport to use for connecting to the debugger (mandatory, possible values: 'dt_socket')
    //     address=<hostname>:<port>    Address to connect to (mandatory)
    //     loglevel=<n>                 Log level (defaults to 0)
    //     logfile=<file>               File to log to (defaults to stdout)
    //     suspend=y/n                  Whether to suspend after startup.
    //     timeout=<n>                  Timeout for connecting in milliseconds.
    //     server=y/n                   Whether to listen for a client connection.
    //     keepalive=<n>                Send keepalive events every n milliseconds.
    //     setpgid=y/n                  Whether to call setpid(0, 0) after startup.
    char* options[] =
    {
        "--soft-breakpoints",
        "--debugger-agent=transport=dt_socket,address=127.0.0.1:55555,loglevel=10,logfile=monoLog.txt,suspend=n,server=y"
        //"--debugger-agent=transport=dt_socket,address=127.0.0.1:55555,loglevel=10,logfile=monoLog.txt,server=y"
    };
    mono_jit_parse_options( 2, options );
    mono_debug_init( MONO_DEBUG_FORMAT_MONO );

    // Create the core mono JIT domain.
    mono_set_dirs( "./mono/lib", "" );
    m_pCoreDomain = mono_jit_init( "MyEngine" );

    m_pDLLFile = nullptr;

    m_pActiveDomain = nullptr;
    m_pMonoImage = nullptr;

#if MYFW_EDITOR
    m_RebuildWhenCompileFinishes = false;
    m_pJob_RebuildDLL = MyNew JobWithCallbackFunction( this, MonoGameState::CompileDLL );
#endif
}

MonoGameState::~MonoGameState()
{
    mono_jit_cleanup( m_pCoreDomain );

    SAFE_RELEASE( m_pDLLFile );

#if MYFW_EDITOR
    delete m_pJob_RebuildDLL;
#endif //MYFW_EDITOR
}

void MonoGameState::SetAsGlobalState()
{
    g_pActiveDomain = m_pActiveDomain;
    g_pMonoImage = m_pMonoImage;
}

#if MYFW_EDITOR
void GetListOfFilesInFolder(std::vector<std::string>* pFileList, const char *name, const char* ext)
{
    DIR* dir;
    struct dirent* entry;

    if( (dir = opendir(name)) == 0 )
        return;

    while( (entry = readdir(dir)) != 0 )
    {
        char path[PATH_MAX];

        if( entry->d_type == DT_DIR )
        {
            if( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 )
                continue;

            sprintf_s( path, sizeof(path), "%s/%s", name, entry->d_name );
            GetListOfFilesInFolder( pFileList, path, ext );
        }
        else
        {
            int len = (int)strlen( entry->d_name );
            int extLen = (int)strlen( ext );
            if( len > extLen )
            {
                if( strcmp( &entry->d_name[len-extLen], ext ) == 0 )
                {
                    sprintf_s( path, sizeof(path), "%s/%s", name, entry->d_name );
                    pFileList->push_back( path );
                }
            }
        }
    }

    closedir(dir);
}

void MonoGameState::CheckForUpdatedScripts()
{
    // If any of the .cs files are newer than the dll, rebuild the dll.
    bool monoDLLNeedsRebuilding = false;

    std::vector<std::string> fileList;
    GetListOfFilesInFolder( &fileList, "DataSource/C#", ".cs" );
    if( fileList.size() > 0 )
    {
        // Add all the internal engine .cs files to the list.
        GetListOfFilesInFolder( &fileList, "DataSource/DataEngineSource/C#", ".cs" );

        FileTimeStamp DLLTimeStamp = GetFileLastModifiedTime( "Data/Mono/Game.dll" );

        for( uint32 i=0; i<fileList.size(); i++ )
        {
            FileTimeStamp fileTimeStamp = GetFileLastModifiedTime( fileList[i].c_str() );

            if( fileTimeStamp > DLLTimeStamp )
            {
                monoDLLNeedsRebuilding = true;
                break;
            }
        }
    }
    else
    {
        // TODO: All .cs files are gone, unload and delete the .dll?
        // It should still build and load with just engine .cs files.
        LOGInfo( LOGTag, "No .cs files found. Old game.dll still in effect.\n" );
    }

    if( monoDLLNeedsRebuilding )
    {
        LOGInfo( LOGTag, "Recompiling Mono DLL.\n" );

        m_pEngineCore->GetManagers()->GetJobManager()->AddJob( m_pJob_RebuildDLL );

        m_RebuildWhenCompileFinishes = true;
    }
}

void MonoGameState::Tick()
{
    if( m_RebuildWhenCompileFinishes )
    {
        if( m_pJob_RebuildDLL->IsFinished() )
        {
            m_RebuildWhenCompileFinishes = false;
            m_pJob_RebuildDLL->Reset();

            LOGInfo( LOGTag, "Compiling finished, loading new DLL.\n" );
        }
    }

    if( m_pMonoImage == nullptr )
    {
        if( m_pDLLFile && m_pDLLFile->IsFinishedLoading() == true )
        {
            if( Rebuild() )
            {
                LOGInfo( LOGTag, "Compiling finished, DLL Loaded and Mono game state built.\n" );
                ComponentMonoScript* pComponent = (ComponentMonoScript*)m_pEngineCore->GetComponentSystemManager()->GetFirstComponentOfType( "MonoScriptComponent" );
                if( pComponent != nullptr )
                {
                    pComponent->LoadScript( true );
                }
            }
        }
    }
}

void MonoGameState::CompileDLL()
{
    std::vector<std::string> output;

    CreateDirectory( "Data/Mono", nullptr );
    LaunchApplication( "C:\\Program Files (x86)\\Mono\\bin\\mcs",
        "-debug /t:library /out:Data/Mono/Game.dll -recurse:DataSource/C#/*.cs -recurse:DataSource/DataEngineSource/C#/*.cs",
        true, false, &output );

    for( std::string str : output )
    {
        LOGInfo( LOGTag, "%s\n", str.c_str() );
    }

    const char* pMonoDLLFilename = "Data/Mono/Game.dll";

#if MYFW_EDITOR
    m_pDLLFile = m_pEngineCore->GetManagers()->GetFileManager()->LoadFileNow( pMonoDLLFilename );
#else
    m_pDLLFile = m_pEngineCore->GetManagers()->GetFileManager()->RequestFile( pMonoDLLFilename );
#endif

    m_pEngineCore->GetManagers()->GetFileManager()->ReloadFileNow( m_pDLLFile );
}

bool MonoGameState::IsRebuilding()
{
    if( m_pJob_RebuildDLL->IsActive() )
        return true;

    return false;
}
#endif //MYFW_EDITOR

bool MonoGameState::Rebuild()
{
    // If the domain has been created and an assembly loaded, then destroy that domain and rebuild.
    if( m_pActiveDomain )
    {
        mono_domain_set( m_pCoreDomain, true );
        MonoObject* pException = nullptr;
        mono_domain_try_unload( m_pActiveDomain, &pException );
        if( pException )
        {
            char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
            LOGError( "MonoScript", "Exception thrown calling mono_domain_try_unload(): %s\n", str );
        }
        m_pActiveDomain = nullptr;
        mono_image_close( m_pMonoImage );
        m_pMonoImage = nullptr;
    }

    const char* pMonoDLLFilename = "Data/Mono/Game.dll";

    MonoAssembly* pMonoAssembly = nullptr;

    // Load the assembly and grab a pointer to the image from the assembly.
    if( false ) // Load DLL directly from disk.
    {
        // Create a domain for the game assembly that we will load.
        m_pActiveDomain = mono_domain_create_appdomain( "TestDomain", nullptr );
        // Set the new domain as active.
        mono_domain_set( m_pActiveDomain, true );

        pMonoAssembly = mono_domain_assembly_open( m_pActiveDomain, pMonoDLLFilename );
        if( pMonoAssembly == nullptr )
        {
            mono_domain_set( m_pCoreDomain, true );
            MonoObject* pException = nullptr;
            mono_domain_try_unload( m_pActiveDomain, &pException );
            if( pException )
            {
                char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
                LOGError( "MonoScript", "Exception thrown calling mono_domain_try_unload(): %s\n", str );
            }
            m_pActiveDomain = nullptr;
            LOGError( LOGTag, "%s not found", pMonoDLLFilename );
            return true;
        }
    }
    else // Load DLL into memory then create image and assembly.
    {
        if( m_pDLLFile == nullptr )
        {
#if MYFW_EDITOR
            m_pDLLFile = m_pEngineCore->GetManagers()->GetFileManager()->LoadFileNow( pMonoDLLFilename );
#else
            m_pDLLFile = m_pEngineCore->GetManagers()->GetFileManager()->RequestFile( pMonoDLLFilename );
#endif
        }

        if( m_pDLLFile == nullptr || m_pDLLFile->IsFinishedLoading() == false )
        {
            return false;
        }

        if( m_pDLLFile->GetFileLoadStatus() == FileLoadStatus_Error_FileNotFound )
        {
            return false;
        }

        MyAssert( m_pDLLFile->GetFileLoadStatus() == FileLoadStatus_Success );

        // Create a domain for the game assembly that we will load.
        m_pActiveDomain = mono_domain_create_appdomain( "TestDomain", nullptr );
        // Set the new domain as active.
        mono_domain_set( m_pActiveDomain, true );

        MonoImageOpenStatus openStatus;
        //LOGInfo( LOGTag, "Creating Mono Image.\n" );
        m_pMonoImage = mono_image_open_from_data( const_cast<char*>(m_pDLLFile->GetBuffer()), m_pDLLFile->GetFileLength(), false, &openStatus );
        MyAssert( openStatus == MONO_IMAGE_OK );
        pMonoAssembly = mono_assembly_load_from_full( m_pMonoImage, "C:\\", &openStatus, false );
        MyAssert( openStatus == MONO_IMAGE_OK );
    }

    MyAssert( pMonoAssembly );
    m_pMonoImage = mono_assembly_get_image( pMonoAssembly );

    SetAsGlobalState();

    // Register Mono interface functions.
    RegisterMonoFrameworkClasses( this );
    RegisterMonoMaterialDefinition( this );
    RegisterMonoMaterialManager( this );
    RegisterMonoComponentSystemManager( this );
    RegisterMonoEngineCore( this );
    RegisterMonoGameObject( this );
    RegisterMonoComponentTransform( this );
    RegisterMonoComponentMesh( this );
    RegisterMonoComponentMeshPrimitive( this );

    return true;
}
