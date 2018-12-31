//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

// TODO: Fix GL Includes.
#include <gl/GL.h>
#include "../../../../Framework/MyFramework/SourceWindows/GLExtensions.h"
#include "../../../../Framework/MyFramework/SourceCommon/Shaders/GLHelpers.h"

#if MYFW_USING_LUA

int MyGetUniformLocation(uint32 program, const char* name);
void MyUniform1f(GLint location, GLfloat v0);
void MyUniform2f(GLint location, GLfloat v0, GLfloat v1);
void MyUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void MyUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void MyUniform1fv(GLint location, GLsizei count, const luabridge::LuaRef value);
void MyUniform2fv(GLint location, GLsizei count, const luabridge::LuaRef value);
void MyUniform3fv(GLint location, GLsizei count, const luabridge::LuaRef value);
void MyUniform4fv(GLint location, GLsizei count, const luabridge::LuaRef value);

void LuaRegisterGLFunctions(lua_State* luastate)
{
    // Register some GL functions.
    luabridge::getGlobalNamespace( luastate )
        .addFunction( "glGetUniformLocation", MyGetUniformLocation ) // int MyGetUniformLocation(uint32 program, const char* name)
        .addFunction( "glUniform1f",  MyUniform1f ) // void MyUniform1f(GLint location, GLfloat v0)
        .addFunction( "glUniform2f",  MyUniform2f ) // void MyUniform2f(GLint location, GLfloat v0, GLfloat v1)
        .addFunction( "glUniform3f",  MyUniform3f ) // void MyUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
        .addFunction( "glUniform4f",  MyUniform4f ) // void MyUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
        .addFunction( "glUniform1fv", MyUniform1fv ) // void MyUniform1fv(GLint location, GLsizei count, const luabridge::LuaRef value)
        .addFunction( "glUniform2fv", MyUniform2fv ) // void MyUniform2fv(GLint location, GLsizei count, const luabridge::LuaRef value)
        .addFunction( "glUniform3fv", MyUniform3fv ) // void MyUniform3fv(GLint location, GLsizei count, const luabridge::LuaRef value)
        .addFunction( "glUniform4fv", MyUniform4fv ); // void MyUniform4fv(GLint location, GLsizei count, const luabridge::LuaRef value)
}

int MyGetUniformLocation(uint32 program, const char* name)
{
    return glGetUniformLocation( program, name );
}

void MyUniform1f(GLint location, GLfloat v0)
{
    return glUniform1f( location, v0 );
}

void MyUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    return glUniform2f( location, v0, v1 );
}

void MyUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    return glUniform3f( location, v0, v1, v2 );
}

void MyUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    return glUniform4f( location, v0, v1, v2, v3 );
}

void MyUniform1fv(GLint location, GLsizei count, const luabridge::LuaRef value)
{
    if( count*1 > value.length() )
    {
        LOGError( LOGTag, "glUniform1fv called with wrong number of floats" );
        return;
    }

    MyStackAllocator::MyStackPointer stackpointer;
    float* values = (float*)g_pEngineCore->GetSingleFrameMemoryStack().AllocateBlock( sizeof(float)*1*count, &stackpointer );

    for( int i=0; i<count*1; i++ )
        values[i] = value[i+1]; // LUA array starts at index 1.

    glUniform1fv( location, count, values );

    g_pEngineCore->GetSingleFrameMemoryStack().RewindStack( stackpointer );
}

void MyUniform2fv(GLint location, GLsizei count, const luabridge::LuaRef value)
{
    if( count > value.length() )
    {
        LOGError( LOGTag, "glUniform2fv called with wrong number of Vector2s" );
        return;
    }

    MyStackAllocator::MyStackPointer stackpointer;
    Vector2* values = (Vector2*)g_pEngineCore->GetSingleFrameMemoryStack().AllocateBlock( sizeof(Vector2)*count, &stackpointer );

    for( int i=0; i<count; i++ )
        values[i] = value[i+1]; // LUA array starts at index 1.

    glUniform2fv( location, count, &values[0].x );

    g_pEngineCore->GetSingleFrameMemoryStack().RewindStack( stackpointer );

    //if( count*2 > value.length() )
    //{
    //    LOGError( LOGTag, "glUniform2fv called with wrong number of floats" );
    //    return;
    //}

    //MyStackAllocator::MyStackPointer stackpointer;
    //float* values = (float*)g_pEngineCore->GetSingleFrameMemoryStack().AllocateBlock( sizeof(float)*2*count, &stackpointer );

    //for( int i=0; i<count*2; i++ )
    //    values[i] = value[i+1]; // LUA array starts at index 1.

    //glUniform2fv( location, count, values );

    //g_pEngineCore->GetSingleFrameMemoryStack().RewindStack( stackpointer );
}

void MyUniform3fv(GLint location, GLsizei count, const luabridge::LuaRef value)
{
    if( count > value.length() )
    {
        LOGError( LOGTag, "glUniform3fv called with wrong number of Vector3s" );
        return;
    }

    MyStackAllocator::MyStackPointer stackpointer;
    Vector3* values = (Vector3*)g_pEngineCore->GetSingleFrameMemoryStack().AllocateBlock( sizeof(Vector3)*count, &stackpointer );

    for( int i=0; i<count; i++ )
        values[i] = value[i+1]; // LUA array starts at index 1.

    glUniform3fv( location, count, &values[0].x );

    g_pEngineCore->GetSingleFrameMemoryStack().RewindStack( stackpointer );

    //if( count*3 > value.length() )
    //{
    //    LOGError( LOGTag, "glUniform3fv called with wrong number of floats" );
    //    return;
    //}

    //MyStackAllocator::MyStackPointer stackpointer;
    //float* values = (float*)g_pEngineCore->GetSingleFrameMemoryStack().AllocateBlock( sizeof(float)*3*count, &stackpointer );

    //for( int i=0; i<count*3; i++ )
    //    values[i] = value[i+1]; // LUA array starts at index 1.

    //glUniform3fv( location, count, values );

    //g_pEngineCore->GetSingleFrameMemoryStack().RewindStack( stackpointer );
}

void MyUniform4fv(GLint location, GLsizei count, const luabridge::LuaRef value)
{
    if( count > value.length() )
    {
        LOGError( LOGTag, "glUniform4fv called with wrong number of Vector4s" );
        return;
    }

    MyStackAllocator::MyStackPointer stackpointer;
    Vector4* values = (Vector4*)g_pEngineCore->GetSingleFrameMemoryStack().AllocateBlock( sizeof(Vector4)*count, &stackpointer );

    for( int i=0; i<count; i++ )
        values[i] = value[i+1]; // LUA array starts at index 1.

    glUniform4fv( location, count, &values[0].x );

    g_pEngineCore->GetSingleFrameMemoryStack().RewindStack( stackpointer );

    //if( count*4 > value.length() )
    //{
    //    LOGError( LOGTag, "glUniform4fv called with wrong number of floats" );
    //    return;
    //}

    //MyStackAllocator::MyStackPointer stackpointer;
    //float* values = (float*)g_pEngineCore->GetSingleFrameMemoryStack().AllocateBlock( sizeof(float)*4*count, &stackpointer );

    //for( int i=0; i<count*4; i++ )
    //    values[i] = value[i+1]; // LUA array starts at index 1.

    //glUniform4fv( location, count, values );

    //g_pEngineCore->GetSingleFrameMemoryStack().RewindStack( stackpointer );
}

#endif //MYFW_USING_LUA
