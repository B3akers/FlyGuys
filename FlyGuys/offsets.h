#pragma once
#include <cinttypes>

namespace signatures {
	enum values {
		game_object_custom_set_active = 0x092B600,
		get_main_camera = 0x0909890,
		swap_chain = 0x0462B20,
		game_object_manager = 0x17B70E8,
		global_game_state = 45847968, //Class$MPG.Utility.Singleton<GlobalGameStateClient>
		catapult_services = 45843544, //Class$FGClient.CatapultServices.CatapultServices
		re_input = 46143608, //Class$Rewired.ReInput
		cursor_manager = 45921472, //Class$CursorManager
		get_bound = 23004128, //UnityEngine.Collider$$get_bounds
	};
};

namespace unity {
	enum {
		native_ptr = 0x10,
		compoment_owner = 0x30,
		components_size = 0x40,
		components_ptr = 0x30,
		mono_ptr = 0x28,
		transform_compoment = 0x8,
	};

	namespace camera {
		enum {
			enabled = 0x38 //Behaviour::GetEnabled
		};
	};
};

class base_object;
class last_object_base;
class game_object {
public:
	const char* get_name( ) {
		return *reinterpret_cast<const char**>( std::uintptr_t( this ) + 0x60 );
	}
	
	int16_t get_object_tag( ) {
		return *reinterpret_cast<int16_t*>( std::uintptr_t( this ) + 0x54 );
	}
};
class game_object_manager {
public:
	last_object_base* last_tagged_object;
	base_object* tagged_objects;
	last_object_base* last_active_object;
	base_object* active_objects;
};
class base_object {
public:
	char pad_0x0000[ 0x8 ];
	base_object* next_node;
	game_object* object;
};
class last_object_base {
public:
	char pad_0x0000[ 0x10 ];
	game_object* last_object;
};
