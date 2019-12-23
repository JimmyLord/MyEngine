//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __InputFinger_H__
#define __InputFinger_H__

#define MAX_FINGERS 10

class InputFinger;
extern InputFinger g_Fingers[MAX_FINGERS];

class InputFinger
{
public:
    float initx;
    float inity;
    float lastx;
    float lasty;
    float currx;
    float curry;
    float storedx;
    float storedy;
    float furthesttravel;
    int id;
    double timepressed;

    InputFinger()
    {
        reset();
    }

    void reset()
    {
        initx = -1000;
        inity = -1000;
        lastx = -1000;
        lasty = -1000;
        currx = -1000;
        curry = -1000;
        storedx = -1000;
        storedy = -1000;
        id = -1;
        timepressed = 0;
        furthesttravel = 0;
    }

    void set(float x, float y, int bid, double time = 0, bool tracktravel = false)
    {
        if( id == -1 )
        {
            initx = x;
            inity = y;
            lastx = x;
            lasty = y;
            id = bid;
            timepressed = time;
            furthesttravel = 0;
        }

        lastx = currx;
        lasty = curry;
        currx = x;
        curry = y;

        if( tracktravel )
        {
            float travel = fabsf(currx-initx) + fabsf(curry-inity);
            if( travel > furthesttravel )
                furthesttravel = travel;
        }
    }
};

extern void ClearAllFingers();
extern void ClearFinger(int index);

#endif //__InputFinger_H__
