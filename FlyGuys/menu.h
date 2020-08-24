/* This file is part of FlyGuys by b3akers, licensed under the MIT license:
*
* MIT License
*
* Copyright (c) b3akers 2020
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once
#include <Windows.h>

namespace settings {
	namespace movement {
		extern bool fly_enabled;
		extern int fly_speed;
		extern bool speed_enabled;
		extern int speed_boost;

		extern bool disable_stun_collision;
		extern bool disable_object_collisions;
	};

	namespace cheat {
		extern bool hide_enabled_cheats;
		extern bool player_esp_enabled;
		extern bool super_grab_enabled;
		extern float grabber_velocity;
	};
};
namespace cheat_helper {
	extern bool disable_fly;
	extern bool disable_speed;

};

namespace menu {
	void draw( );
	void update_keys( );
	void update_indicators( );
};

