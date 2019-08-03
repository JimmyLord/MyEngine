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
//#include "ComponentSystem/BaseComponents/ComponentBase.h"
//#include "ComponentSystem/BaseComponents/ComponentGameObjectProperties.h"
//#include "ComponentSystem/BaseComponents/ComponentMenuPage.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
//#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
//#include "ComponentSystem/EngineComponents/ComponentHeightmap.h"
//#if MYFW_USING_MONO
//#include "ComponentSystem/EngineComponents/ComponentMonoScript.h"
//#endif //MYFW_USING_MONO
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

MonoGameState::MonoGameState(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    // Create the core mono JIT domain.
    mono_set_dirs( "./mono/lib", "" );
    m_pCoreDomain = mono_jit_init( "MyEngine" );

    m_pActiveDomain = nullptr;
    m_pMonoImage = nullptr;
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
    if( pMonoAssembly == nullptr )
    {
        mono_domain_set( m_pCoreDomain, true );
        mono_domain_unload( m_pActiveDomain );
        LOGError( LOGTag, "%s not found", "Data/Mono/Game.dll" );
        return;
    }

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
