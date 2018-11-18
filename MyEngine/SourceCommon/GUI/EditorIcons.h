//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorIcons_H__
#define __EditorIcons_H__

// UTF8 reference:
// Unicode value     UTF8 byte code
//                   1st byte - 2nd byte - 3rd byte
// U+E000..U+FFFF    EE..EF     80..BF     80..BF

// OpenIconsFont
#define EditorIcon_GameObject   "\xEE\x80\xA5" // 0xe025 box
//                              "\xEE\x81\x8b" // 0xe04b cpu
#define EditorIcon_Folder       "\xEE\x81\xA7" // 0xe067 folder
#define EditorIcon_Prefab       "\xEE\x82\x9b" // 0xe09b package

#endif //__EditorIcons_H__
