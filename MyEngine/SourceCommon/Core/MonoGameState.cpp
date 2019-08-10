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
#include "Core/EngineCore.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
//#include "ComponentSystem/BaseComponents/ComponentBase.h"
//#include "ComponentSystem/BaseComponents/ComponentGameObjectProperties.h"
//#include "ComponentSystem/BaseComponents/ComponentMenuPage.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
//#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
//#include "ComponentSystem/EngineComponents/ComponentHeightmap.h"
#if MYFW_USING_MONO
#include "ComponentSystem/EngineComponents/ComponentMonoScript.h"
#endif //MYFW_USING_MONO
//#include "ComponentSystem/EngineComponents/ComponentObjectPool.h"
//#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer.h"
//#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer2D.h"
//#include "ComponentSystem/FrameworkComponents/ComponentAudioPlayer.h"
//#if MYFW_USING_LUA
//#include "ComponentSystem/FrameworkComponents/ComponentLuaScript.h"
//#endif //MYFW_USING_LUA
//#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
//#include "ComponentSystem/FrameworkComponents/ComponentMeshPrimitive.h"
//#include "ComponentSystem/FrameworkComponents/ComponentParticleEmitter.h"
//#include "ComponentSystem/FrameworkComponents/ComponentSprite.h"
//#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DCollisionObject.h"
//#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"
//#include "Voxels/ComponentVoxelMesh.h"
//#include "Voxels/ComponentVoxelWorld.h"

#if MYFW_EDITOR
#if MYFW_WINDOWS
#include "../../Libraries/dirent/dirent.h"
#else
#include <dirent.h>
#endif
#endif

Vector3 g_Vector( 111, 222, 333 );

MonoDomain* g_pActiveDomain; // HACK: REMOVE ME.
MonoImage* g_pMonoImage; // HACK: REMOVE ME.

MonoObject* Mono_GetPosition(GameObject* pGameObject)
{
    MonoClass* pVec3Class = mono_class_from_name( g_pMonoImage, "MyEngine", "vec3" );
    MonoObject* pVec3Instance = mono_object_new( g_pActiveDomain, pVec3Class );

    MonoMethod* pConstructor = mono_class_get_method_from_name( pVec3Class, ".ctor", 3 );
    Vector3 pos = pGameObject->GetTransform()->GetLocalPosition();
    void* args[3];
    args[0] = &pos.x;
    args[1] = &pos.y;
    args[2] = &pos.z;
    mono_runtime_invoke( pConstructor, pVec3Instance, args, nullptr );

    //mono_runtime_object_init( pVec3Instance );
    //g_Vector = pGameObject->GetTransform()->GetLocalPosition();
    //void* value = &g_Vector;
    //MonoClassField* pNativeObjectField = mono_class_get_field_from_name( pVec3Class, "m_pNativeObject" );
    //mono_field_set_value( pVec3Instance, pNativeObjectField, &value );

    return pVec3Instance;
}

void Mono_SetPosition(GameObject* pGameObject, float x, float y, float z)
{
    int bp = 1;

    //void* pData = mono_object_unbox( pVec3 );
    //Vector3* pVector3 = (Vector3*)pData;

    pGameObject->GetTransform()->SetLocalPosition( Vector3( x, y, z ) );
}

MonoObject* Mono_GetVector3()
{
    MonoClass* pVec3Class = mono_class_from_name( g_pMonoImage, "MyEngine", "vec3" );
    MonoClassField* pNativeObjectField = mono_class_get_field_from_name( pVec3Class, "m_pNativeObject" );

    MonoObject* pManagedObject = mono_object_new( g_pActiveDomain, pVec3Class );
    mono_runtime_object_init( pManagedObject );
    void* value = &g_Vector;
    mono_field_set_value( pManagedObject, pNativeObjectField, &value );

    return pManagedObject;
}

float Mono_GetX(Vector3* pVec3) { return pVec3->x; }
float Mono_GetY(Vector3* pVec3) { return pVec3->y; }
float Mono_GetZ(Vector3* pVec3) { return pVec3->z; }

void Mono_LOGInfo(MonoString* monoStr)
{
    char* str = mono_string_to_utf8( monoStr );
    LOGInfo( "MonoLog", "Received string: %s", str );
}

void Mono_LOGError(MonoString* monoStr)
{
    char* str = mono_string_to_utf8( monoStr );
    LOGError( "MonoLog", "Received string: %s", str );
}

#if MYFW_EDITOR
class Job_RebuildDLL : public MyJob
{
protected:
    MonoGameState* m_pMonoGameState;

public:
    Job_RebuildDLL(MonoGameState* pMonoGameState)
    {
        m_pMonoGameState = pMonoGameState;
    }
    virtual ~Job_RebuildDLL() {}

    virtual void DoWork()
    {
        m_pMonoGameState->CompileDLL();
    }
};
#endif

MonoGameState::MonoGameState(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    // Create the core mono JIT domain.
    mono_set_dirs( "./mono/lib", "" );
    m_pCoreDomain = mono_jit_init( "MyEngine" );

    m_pDLLFile = nullptr;

    m_pActiveDomain = nullptr;
    m_pMonoImage = nullptr;

#if MYFW_EDITOR
    m_RebuildWhenCompileFinishes = false;
    m_pJob_RebuildDLL = MyNew Job_RebuildDLL( this );
#endif
}

MonoGameState::~MonoGameState()
{
    mono_jit_cleanup( m_pCoreDomain );

    SAFE_RELEASE( m_pDLLFile );

    delete m_pJob_RebuildDLL;
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
            int len = strlen( entry->d_name );
            int extLen = strlen( ext );
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

            LOGInfo( LOGTag, "Compiling finished, rebuilding Mono game state.\n" );
            Rebuild();

            ComponentMonoScript* pComponent = (ComponentMonoScript*)m_pEngineCore->GetComponentSystemManager()->GetFirstComponentOfType( "MonoScriptComponent" );
            pComponent->LoadScript( true );
        }
    }
}

void MonoGameState::CompileDLL()
{
    std::vector<std::string> output;

    LaunchApplication( "C:\\Program Files (x86)\\Mono\\bin\\csc",
        "/t:library /out:Data/Mono/Game.dll DataSource/C#/*.cs DataSource/DataEngineSource/C#/*.cs",
        true, false, &output );

    for( std::string str : output )
    {
        LOGInfo( LOGTag, "%s\n", str.c_str() );
    }

    m_pEngineCore->GetManagers()->GetFileManager()->ReloadFileNow( m_pDLLFile );
}

bool MonoGameState::IsRebuilding()
{
    if( m_pJob_RebuildDLL->IsActive() )
        return true;

    return false;
}
#endif //MYFW_EDITOR

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

    const char* pMonoDLLFilename = "Data/Mono/Game.dll";

    MonoAssembly* pMonoAssembly = nullptr;

    // Load the assembly and grab a pointer to the image from the assembly.
    if( false ) // Load DLL directly from disk.
    {
        pMonoAssembly = mono_domain_assembly_open( m_pActiveDomain, pMonoDLLFilename );
        if( pMonoAssembly == nullptr )
        {
            mono_domain_set( m_pCoreDomain, true );
            mono_domain_unload( m_pActiveDomain );
            m_pActiveDomain = nullptr;
            LOGError( LOGTag, "%s not found", pMonoDLLFilename );
            return;
        }
    }
    else // Load DLL into memory then create image and assembly.
    {
        if( m_pDLLFile == 0 )
        {
            m_pDLLFile = m_pEngineCore->GetManagers()->GetFileManager()->LoadFileNow( pMonoDLLFilename );
        }

        if( m_pDLLFile == nullptr )
        {
            mono_domain_set( m_pCoreDomain, true );
            mono_domain_unload( m_pActiveDomain );
            m_pActiveDomain = nullptr;
            LOGError( LOGTag, "%s not found", pMonoDLLFilename );
            return;
        }

        MonoImageOpenStatus openStatus;
        //LOGInfo( LOGTag, "Creating Mono Image.\n" );
        m_pMonoImage = mono_image_open_from_data( const_cast<char*>(m_pDLLFile->GetBuffer()), m_pDLLFile->GetFileLength(), false, &openStatus );
        MyAssert( openStatus == MONO_IMAGE_OK );
        pMonoAssembly = mono_assembly_load_from_full( m_pMonoImage, "C:\\", &openStatus, false );
        MyAssert( openStatus == MONO_IMAGE_OK );
    }

    MyAssert( pMonoAssembly );
    m_pMonoImage = mono_assembly_get_image( pMonoAssembly );

    g_pActiveDomain = m_pActiveDomain; // HACK: REMOVE ME.
    g_pMonoImage = m_pMonoImage; // HACK: REMOVE ME.

    // Register some global functions.
    mono_add_internal_call( "MyEngine.Log::Info", Mono_LOGInfo );
    mono_add_internal_call( "MyEngine.Log::Error", Mono_LOGError );

    mono_add_internal_call( "MyEngine.vec3::GetVector", Mono_GetVector3 );
    mono_add_internal_call( "MyEngine.vec3::GetX", Mono_GetX );
    mono_add_internal_call( "MyEngine.vec3::GetY", Mono_GetY );
    mono_add_internal_call( "MyEngine.vec3::GetZ", Mono_GetZ );

    //GameObject::LuaRegister( nullptr );
    mono_add_internal_call( "MyEngine.GameObject::GetPosition", Mono_GetPosition );
    mono_add_internal_call( "MyEngine.GameObject::SetPosition", Mono_SetPosition );
}
