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
		extern bool make_me_reach;
		extern bool player_esp_enabled;
		extern bool super_grab_enabled;
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

