#include "menu.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "draw_manager.h"
#include "render.h"

#include <mutex>

namespace settings {
	namespace movement {
		bool fly_enabled = false;
		int fly_speed = 30;
		bool speed_enabled = false;
		int speed_boost = 15;

		bool disable_stun_collision = false;
		bool disable_object_collisions = false;
	};
	namespace cheat {
		bool make_me_reach = false;
		bool player_esp_enabled = false;
		bool super_grab_enabled = false;
	};
};
namespace cheat_helper {
	bool disable_fly = false;
	bool disable_speed = false;
};

namespace menu {
	bool movement_tab_active = true;
	bool misc_tab_active = true;

	void push_color_for_button( bool active ) {
		if ( active ) {
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4 { 0.f, 92.f / 255.f, 196.f / 255.f, 1.f } );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4 { 0.f, 92.f / 255.f, 196.f / 255.f, 1.f } );
		} else {
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4 { 61.f / 255.f, 61.f / 255.f, 61.f / 255.f, 1.f } );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4 { 0.f, 128.f / 255.f, 199.f / 255.f, 1.f } );
		}
	}

	void draw_button( const char* name, bool& config_key, bool* change_opositive = nullptr ) {
		push_color_for_button( config_key );
		if ( ImGui::Button( name, { 125,20 } ) ) {
			config_key = !config_key;
			if ( change_opositive )
				*change_opositive = !config_key;
		}

		ImGui::PopStyleColor( 2 );
	}

	void draw_slider( const char* name, int* val, int min, int max ) {
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4 { 0.f, 92.f / 255.f, 196.f / 255.f, 1.f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4 { 0.f, 92.f / 255.f, 196.f / 255.f, 1.f } );

		ImGui::SliderInt( name, val, min, max );

		ImGui::PopStyleColor( 2 );
	}

	void draw_tab( const char* name, bool& active ) {
		ImGui::Text( name );
		ImGui::SameLine( 118 );

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4 { 0.f, 92.f / 255.f, 196.f / 255.f, 1.f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4 { 0.f, 92.f / 255.f, 196.f / 255.f, 1.f } );
		if ( ImGui::Button( "", { 10,10 } ) )
			active = !active;
		ImGui::PopStyleColor( 2 );
	}

	bool collisions_tab_active = true;
	bool tetetete;

	std::once_flag init_style;
	void draw( ) {

		std::call_once( init_style, [ & ] ( ) {
			auto& style = ImGui::GetStyle( );
			style.WindowRounding = 0.f;
			style.FrameRounding = 0.f;
			style.ItemSpacing = ImVec2( 8, 2 );
			style.WindowPadding = ImVec2( 3.f, 3.f );
			style.Colors[ ImGuiCol_FrameBg ] = ImVec4 { 56.f / 255.f, 59.f / 255.f, 58.f / 255.f, 1.f };
			style.Colors[ ImGuiCol_FrameBgActive ] = ImVec4 { 56.f / 255.f, 59.f / 255.f, 58.f / 255.f, 1.f };
			style.Colors[ ImGuiCol_FrameBgHovered ] = ImVec4 { 56.f / 255.f, 59.f / 255.f, 58.f / 255.f, 1.f };
			style.Colors[ ImGuiCol_SliderGrabActive ] = ImVec4 { 0.f, 92.f / 255.f, 196.f / 255.f, 1.f };
			style.Colors[ ImGuiCol_SliderGrab ] = ImVec4 { 0.f, 128.f / 255.f, 199.f / 255.f, 1.f };
			} );

		ImGui::Begin( "tab_movement", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
		ImGui::SetWindowSize( { 131,0 }, ImGuiCond_Always );
		draw_tab( "Movement", movement_tab_active );
		if ( movement_tab_active ) {
			draw_button( "Fly", settings::movement::fly_enabled, &cheat_helper::disable_fly );
			draw_slider( "Fly speed", &settings::movement::fly_speed, 1, 100 );
			draw_button( "Speed", settings::movement::speed_enabled, &cheat_helper::disable_speed );
			draw_slider( "Speed boost", &settings::movement::speed_boost, 1, 100 );
		}
		ImGui::End( );

		ImGui::Begin( "tab_collisions", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
		ImGui::SetWindowSize( { 131,0 }, ImGuiCond_Always );
		draw_tab( "Collisions", collisions_tab_active );
		if ( collisions_tab_active ) {
			draw_button( "Disable stun", settings::movement::disable_stun_collision );
			draw_button( "Disable collisions", settings::movement::disable_object_collisions );
		}
		ImGui::End( );

		ImGui::Begin( "tab_misc", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
		ImGui::SetWindowSize( { 131,0 }, ImGuiCond_Always );
		draw_tab( "Misc", misc_tab_active );
		if ( misc_tab_active ) {
			//draw_button( "Make me rich", settings::cheat::make_me_reach );
			draw_button("Player ESP", settings::cheat::player_esp_enabled);
			draw_button("Super grab", settings::cheat::super_grab_enabled);
		}
		ImGui::End( );
	}

	bool OldKeysDown[ 512 ];
	float OldNavInputs[ ImGuiNavInput_COUNT ];
	void update_keys( ) {

		auto io = ImGui::GetIO( );

		if ( ( io.KeysDown[ VK_INSERT ] && !OldKeysDown[ VK_INSERT ] ) ||
			( io.NavInputs[ ImGuiNavInput_RightThumb ] > 0.f && OldNavInputs[ ImGuiNavInput_RightThumb ] == 0.f ) ) {
			render::menu_is_open = !render::menu_is_open;
		}

		if ( render::menu_is_open ) {
			auto context = ImGui::GetCurrentContext( );
			if ( context->NavWindow ) {
				if ( context->Windows.Size > 1 ) {
					int32_t move_window_direction = -1;
					if ( io.NavInputs[ ImGuiNavInput_FocusPrev ] > 0.f && OldNavInputs[ ImGuiNavInput_FocusPrev ] == 0.f )
						move_window_direction = 0;
					else if ( io.NavInputs[ ImGuiNavInput_FocusNext ] > 0.f && OldNavInputs[ ImGuiNavInput_FocusNext ] == 0.f )
						move_window_direction = 1;

					if ( move_window_direction != -1 ) {
						ImGuiWindow* new_window = nullptr;
						for ( auto window : context->Windows ) {
							if ( window == context->NavWindow || window->Hidden || window->IsFallbackWindow )
								continue;

							if ( !strcmp( window->Name, "BackBuffer" ) )
								continue;

							if ( move_window_direction == 0 ) {
								if ( window->Pos.x <= context->NavWindow->Pos.x )
									if ( !new_window || window->Pos.x > new_window->Pos.x )
										new_window = window;
							} else {
								if ( window->Pos.x >= context->NavWindow->Pos.x )
									if ( !new_window || window->Pos.x < new_window->Pos.x )
										new_window = window;
							}
						}
						if ( new_window ) {
							ImGui::FocusWindow( new_window );
						}
					}
				}
			}
		}

		if ( ( io.KeysDown[ 0x52 ] && !OldKeysDown[ 0x52 ] ) || ( io.NavInputs[ ImGuiNavInput_Input ] && !OldNavInputs[ ImGuiNavInput_Input ] ) ) {
			settings::movement::fly_enabled = !settings::movement::fly_enabled;
			cheat_helper::disable_fly = !settings::movement::fly_enabled;
		}
		if ( io.KeysDown[ 0x43 ] && !OldKeysDown[ 0x43 ] ) {
			settings::movement::speed_enabled = !settings::movement::speed_enabled;
			cheat_helper::disable_speed = !settings::movement::speed_enabled;
		}

		if (io.KeysDown[0x58] && !OldKeysDown[0x58]) {
			settings::cheat::player_esp_enabled = !settings::cheat::player_esp_enabled;
		}

		if (io.KeysDown[0x47] && !OldKeysDown[0x47]) {
			settings::cheat::super_grab_enabled = !settings::cheat::super_grab_enabled;
		}

		if ( io.NavInputs[ ImGuiNavInput_FocusPrev ] > 0.f ) {
			settings::movement::speed_enabled = true;
			cheat_helper::disable_speed = false;
		} else if ( io.NavInputs[ ImGuiNavInput_FocusPrev ] == 0.f && OldNavInputs[ ImGuiNavInput_FocusPrev ] > 0.f ) {
			settings::movement::speed_enabled = false;
			cheat_helper::disable_speed = true;
		}

		memcpy( OldKeysDown, io.KeysDown, 512 * sizeof( bool ) );
		memcpy( OldNavInputs, io.NavInputs, ImGuiNavInput_COUNT * sizeof( float ) );
	}

	void update_indicators( ) {
		auto text_size = draw_manager::calc_text_size( 18, "TEST" );
		auto y = float( 10 );

		if ( settings::movement::fly_enabled ) {
			draw_manager::add_text_on_screen( { 10,y }, 0xFFFFFFFF, 18, "Fly [R]" );
			y += text_size.y + 4.f;
		}
		if ( settings::movement::speed_enabled ) {
			draw_manager::add_text_on_screen( { 10,y }, 0xFFFFFFFF, 18, "Speed [C]" );
			y += text_size.y + 4.f;
		}

		if (settings::cheat::player_esp_enabled) {
			draw_manager::add_text_on_screen({ 10,y }, 0xFFFFFFFF, 18, "Player ESP [X]");\
			y += text_size.y + 4.f;
		}

		if (settings::cheat::super_grab_enabled) {
			draw_manager::add_text_on_screen({ 10,y }, 0xFFFFFFFF, 18, "Super Grab [G]");
			y += text_size.y + 4.f;
		}
		if ( settings::movement::disable_stun_collision ) {
			draw_manager::add_text_on_screen( { 10,y }, 0xFFFFFFFF, 18, "No Stun" );
			y += text_size.y + 4.f;
		}
		if ( settings::movement::disable_object_collisions ) {
			draw_manager::add_text_on_screen( { 10,y }, 0xFFFFFFFF, 18, "No Collisions" );
			y += text_size.y + 4.f;
		}
	}
};
