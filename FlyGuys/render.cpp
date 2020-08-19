#include "render.h"
#include "offsets.h"
#include "console.h"

#include <cinttypes>
#include <d3d11.h>
#include <memory>
#include <mutex>
#include <thread>

#include <emmintrin.h>
#include <xmmintrin.h>

#include "VMTHook.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_extend.h"
#include "draw_manager.h"
#include "unity_sdk.h"
#include "fnv_hash.hpp"
#include "menu.h"

#include "directx_helper.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

HWND hwnd;

IDXGISwapChain* swap_chain;
ID3D11DeviceContext* d3d11_device_context;
ID3D11Device* d3d11_device;

std::unique_ptr<vmt_smart_hook> swap_chain_vmt = nullptr;

bool render::menu_is_open = false;
std::once_flag init_device;

const float default_max_speed = 9.500000f;

const float default_anyCollisionStunForce = 28.000000f;
const float default_dynamicCollisionStunForce = 10.000000f;
const float default_CollisionThreshold = 14.000000f;
const float default_carryMaxSpeed = 8.000000f;
const float default_carryPickupDuration = 0.100000f;

const float default_playerGrabDetectRadius = 6.0f;
const float default_playerGrabCheckDistance = 2.0f;
const float default_playerGrabberMaxForce = 0.6000000238f;
const float default_playerGrabBreakTime = 1.200000048f;
const float default_armLength = 1.0f;
const float default_playerGrabCheckPredictionBase = 0.1000000015f;
const float default_playerGrabImmediateVelocityReduction = 0.5f;
const float default_playerGrabberDragDirectionContribution = 0.50f;
const float default_grabCooldown = 0.5f;
const float default_playerGrabRegrabDelay = 2.0f;
const float default_playerGrabBreakTimeJumpInfluence = 0.01999999955f;
const float default_forceReleaseRegrabCooldown = 1.0f;
const float default_breakGrabAngle = 75.0f;
const float default_playerGrabbeeMaxForce = 1.0f;
const float default_playerGrabBreakSeparationForce = 7.0f;
const float default_playerGrabbeeInvulnerabilityWindow = 1.5f;

namespace game {
	uintptr_t game = 0;
	uintptr_t unity = 0;
	uintptr_t main_camera = 0;
}

bool world_to_screen( const vector& world, vector& screen_out ) {
	D3DXMATRIX m_world;
	d3d_helper::tmpD3DXMatrixIdentity( &m_world );

	auto& io = ImGui::GetIO( );

	auto w = io.DisplaySize.x;
	auto h = io.DisplaySize.y;

	D3DVIEWPORT9 v_viewport;
	v_viewport.X = 0;
	v_viewport.Y = 0;
	v_viewport.Width = w;
	v_viewport.Height = h;
	v_viewport.MinZ = 1.f;
	v_viewport.MaxZ = 0.f;

	auto screen_v = D3DXVECTOR3( );
	auto world_v = D3DXVECTOR3( );
	world_v.x = world.x;
	world_v.y = world.z;
	world_v.z = world.y;

	auto projection_matrix = reinterpret_cast<D3DMATRIX*>( game::main_camera + 0x9C ); //Camera::GetProjectionMatrix
	auto view_matrix = reinterpret_cast<D3DMATRIX*>( game::main_camera + 0x5C ); //Camera::GetWorldToCameraMatrix

	d3d_helper::tmpD3DXVec3Project( &screen_v, &world_v, &v_viewport, projection_matrix, view_matrix, &m_world );

	if ( screen_v.z < 0.0001f ) {
		screen_out.x = -1;
		screen_out.y = -1;
		return false;
	}

	screen_out.x = screen_v.x / w * w;
	screen_out.y = screen_v.y / h * h;

	return true;
}


bool get_bounding_box2d( UnityEngine_Collider_o* collider, vector& vec_min, vector& vec_max ) {
	auto bounds = UnityEngine_Bounds_o( ); reinterpret_cast<UnityEngine_Bounds_o* ( __stdcall* )( UnityEngine_Bounds_o*, UnityEngine_Collider_o*, void* )>( game::game + signatures::get_bound )( &bounds, collider, 0 );
	auto position = *reinterpret_cast<vector*>( &bounds.fields.m_Center );
	auto bounding_box = *reinterpret_cast<vector*>( &bounds.fields.m_Extents );

	vector world_pos[ 8 ];
	vector screen_pos[ 8 ];
	for ( size_t i = 0; i < 8; i++ )
		world_pos[ i ] = position;

	world_pos[ 0 ].x += bounding_box.x;
	world_pos[ 0 ].y += bounding_box.y;

	world_pos[ 1 ].x -= bounding_box.x;
	world_pos[ 1 ].y += bounding_box.y;

	world_pos[ 2 ].x += bounding_box.x;
	world_pos[ 2 ].y -= bounding_box.y;

	world_pos[ 3 ].x -= bounding_box.x;
	world_pos[ 3 ].y -= bounding_box.y;

	for ( size_t i = 0; i < 4; i++ )
		world_pos[ 4 + i ] = world_pos[ i ];
	for ( size_t i = 0; i < 4; i++ )
		world_pos[ i ].z -= bounding_box.z;
	for ( size_t i = 0; i < 4; i++ )
		world_pos[ 4 + i ].z += bounding_box.z;

	for ( size_t i = 0; i < 8; i++ )
		if ( !world_to_screen( world_pos[ i ], screen_pos[ i ] ) )
			return false;

	vec_min = screen_pos[ 0 ];
	vec_max = vec_min;
	for ( size_t i = 0; i < 8; i++ ) {
		const auto& _x = screen_pos[ i ].x;
		const auto& _y = screen_pos[ i ].y;
		if ( _x < vec_min.x )
			vec_min.x = _x;
		if ( _y < vec_min.y )
			vec_min.y = _y;
		if ( _x > vec_max.x )
			vec_max.x = _x;
		if ( _y > vec_max.y )
			vec_max.y = _y;
	}

	return true;
}

vector get_forward( uint64_t transform_internal ) {
	auto some_ptr = *reinterpret_cast<uint64_t*>( transform_internal + 0x38 );
	auto index = *reinterpret_cast<int32_t*>( transform_internal + 0x38 + sizeof( uint64_t ) );
	if ( !some_ptr )
		return vector( );

	auto relation_array = *reinterpret_cast<uint64_t*>( some_ptr + 0x18 );
	if ( !relation_array )
		return vector( );

	auto dependency_index_array = *reinterpret_cast<uint64_t*>( some_ptr + 0x20 );
	if ( !dependency_index_array )
		return vector( );

	__m128i temp_0;
	__m128i temp_1;
	__m128 temp_2;
	__m128 temp_3;

	__m128 temp_4;
	__m128i temp_5;
	__m128i temp_6;
	__m128 temp_7;

	__m128 xmmword_142E890 = { 0.f, 0.f, 0.f, 0.f };
	__m128 xmmword_142E830 = { -1.f, -1.f, -1.f, 0.f };
	__m128 xmmword_142E8A0 = { 0.f, 0.f, 0.f, FLT_MIN };
	__m128 xmmword_142CC80 = { 1.f, 1.f, 1.f, 1.f };

	auto temp_main = *reinterpret_cast<__m128i*>( relation_array + index * 48 + 16 );
	auto dependency_index = *reinterpret_cast<int32_t*>( dependency_index_array + 4 * index );

	if ( dependency_index >= 0 ) {
		temp_0 = _mm_load_si128( ( const __m128i* ) & xmmword_142E890 );
		temp_1 = _mm_and_si128( _mm_castps_si128( xmmword_142E830 ), temp_0 );
		temp_2 = _mm_castsi128_ps( _mm_load_si128( ( const __m128i* ) & xmmword_142E8A0 ) );
		temp_3 = _mm_and_ps( _mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( xmmword_142E890 ), 0 ) ), temp_2 );

		do {
			auto relation_index = 6 * dependency_index;
			temp_4 = *reinterpret_cast<__m128*>( relation_array + 8 * relation_index + 16 );
			temp_5 = _mm_loadu_si128( ( const __m128i* )( relation_array + 8 * relation_index + 32 ) );
			dependency_index = *reinterpret_cast <int32_t*>( dependency_index_array + 4 * dependency_index );
			temp_6 = _mm_xor_si128( _mm_and_si128( temp_5, temp_0 ), _mm_castps_si128( xmmword_142CC80 ) );
			temp_7 = _mm_castsi128_ps( _mm_xor_si128(
				_mm_and_si128(
					temp_0,
					_mm_castps_si128( _mm_or_ps(
						_mm_andnot_ps(
							temp_2,
							_mm_mul_ps(
								_mm_castsi128_ps( _mm_shuffle_epi32( temp_6, 65 ) ),
								_mm_castsi128_ps( _mm_shuffle_epi32( temp_6, -102 ) ) ) ),
						temp_3 ) ) ),
				temp_main ) );
			temp_main = _mm_xor_si128(
				temp_1,
				_mm_shuffle_epi32(
					_mm_castps_si128( _mm_sub_ps(
						_mm_sub_ps(
							_mm_sub_ps(
								_mm_mul_ps( temp_4, _mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( temp_7 ), -46 ) ) ),
								_mm_castsi128_ps( _mm_shuffle_epi32(
									_mm_castps_si128( _mm_mul_ps(
										temp_4,
										_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( temp_7 ), -120 ) ) ) ),
									30 ) ) ),
							_mm_castsi128_ps( _mm_shuffle_epi32(
								_mm_castps_si128( _mm_mul_ps(
									_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( temp_4 ), -81 ) ),
									temp_7 ) ),
								-115 ) ) ),
						_mm_castsi128_ps( _mm_shuffle_epi32(
							_mm_castps_si128( _mm_mul_ps(
								_mm_movelh_ps( temp_4, temp_4 ),
								_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( temp_7 ), -11 ) ) ) ),
							99 ) ) ) ),
					-46 ) );
		} while ( dependency_index >= 0 );
	}
	auto quad = *reinterpret_cast<quaternion*>( &temp_main );
	return vector( 0.f, 1.f, 0.f ) * quad;
}

vector get_position( uint64_t transform_internal ) {
	auto some_ptr = *reinterpret_cast<uint64_t*>( transform_internal + 0x38 );
	auto index = *reinterpret_cast<int32_t*>( transform_internal + 0x38 + sizeof( uint64_t ) );
	if ( !some_ptr )
		return vector( );

	auto relation_array = *reinterpret_cast<uint64_t*>( some_ptr + 0x18 );
	if ( !relation_array )
		return vector( );

	auto dependency_index_array = *reinterpret_cast<uint64_t*>( some_ptr + 0x20 );
	if ( !dependency_index_array )
		return vector( );

	__m128i temp_0;
	__m128 xmmword_1410D1340 = { -2.f, 2.f, -2.f, 0.f };
	__m128 xmmword_1410D1350 = { 2.f, -2.f, -2.f, 0.f };
	__m128 xmmword_1410D1360 = { -2.f, -2.f, 2.f, 0.f };
	__m128 temp_1;
	__m128 temp_2;
	auto temp_main = *reinterpret_cast<__m128*>( relation_array + index * 48 );
	auto dependency_index = *reinterpret_cast<int32_t*>( dependency_index_array + 4 * index );

	while ( dependency_index >= 0 ) {
		auto relation_index = 6 * dependency_index;

		temp_0 = *reinterpret_cast<__m128i*>( relation_array + 8 * relation_index + 16 );
		temp_1 = *reinterpret_cast<__m128*>( relation_array + 8 * relation_index + 32 );
		temp_2 = *reinterpret_cast<__m128*>( relation_array + 8 * relation_index );

		__m128 v10 = _mm_mul_ps( temp_1, temp_main );
		__m128 v11 = _mm_castsi128_ps( _mm_shuffle_epi32( temp_0, 0 ) );
		__m128 v12 = _mm_castsi128_ps( _mm_shuffle_epi32( temp_0, 85 ) );
		__m128 v13 = _mm_castsi128_ps( _mm_shuffle_epi32( temp_0, -114 ) );
		__m128 v14 = _mm_castsi128_ps( _mm_shuffle_epi32( temp_0, -37 ) );
		__m128 v15 = _mm_castsi128_ps( _mm_shuffle_epi32( temp_0, -86 ) );
		__m128 v16 = _mm_castsi128_ps( _mm_shuffle_epi32( temp_0, 113 ) );
		__m128 v17 = _mm_add_ps(
			_mm_add_ps(
				_mm_add_ps(
					_mm_mul_ps(
						_mm_sub_ps(
							_mm_mul_ps( _mm_mul_ps( v11, xmmword_1410D1350 ), v13 ),
							_mm_mul_ps( _mm_mul_ps( v12, xmmword_1410D1360 ), v14 ) ),
						_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( v10 ), -86 ) ) ),
					_mm_mul_ps(
						_mm_sub_ps(
							_mm_mul_ps( _mm_mul_ps( v15, xmmword_1410D1360 ), v14 ),
							_mm_mul_ps( _mm_mul_ps( v11, xmmword_1410D1340 ), v16 ) ),
						_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( v10 ), 85 ) ) ) ),
				_mm_add_ps(
					_mm_mul_ps(
						_mm_sub_ps(
							_mm_mul_ps( _mm_mul_ps( v12, xmmword_1410D1340 ), v16 ),
							_mm_mul_ps( _mm_mul_ps( v15, xmmword_1410D1350 ), v13 ) ),
						_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( v10 ), 0 ) ) ),
					v10 ) ),
			temp_2 );

		temp_main = v17;
		dependency_index = *reinterpret_cast<int32_t*>( dependency_index_array + 4 * dependency_index );
	}

	return *reinterpret_cast<vector*>( &temp_main );
}

void update( ) {

	game::game = std::uintptr_t( GetModuleHandle( "GameAssembly.dll" ) );
	if ( !game::game )
		return;

	game::unity = std::uintptr_t( GetModuleHandle( "UnityPlayer.dll" ) );
	if ( !game::unity )
		return;

	auto re_input = *reinterpret_cast<Rewired_ReInput_c**>( game::game + signatures::re_input );
	if ( !re_input )
		return;

	auto cursor_manager = *reinterpret_cast<CursorManager_c**>( game::game + signatures::cursor_manager );
	if ( !cursor_manager )
		return;

	if ( render::menu_is_open ) {
		cursor_manager->static_fields->_Instance_k__BackingField->fields.usingKeyboard = true;
		cursor_manager->static_fields->_Instance_k__BackingField->fields.cursorVisible = true;
	}

	re_input->static_fields->HQLaKohzXRajMoKwSNuhmhfTAmCU = render::menu_is_open; //Rewired_ReInput__get_inputAllowed

	auto global = *reinterpret_cast<MPG_Utility_Singleton_GlobalGameStateClient__c**>( game::game + signatures::global_game_state );
	if ( !global )
		return;

	auto instance = global->static_fields->s_Instance;
	if ( !instance || reinterpret_cast<Il2CppClass*>( instance->klass ) != instance->klass->_1.klass )
		return;

	auto game_state_machine = instance->fields._gameStateMachine;
	if ( !game_state_machine )
		return;

	auto current_state = game_state_machine->fields._currentState;
	if ( !current_state || reinterpret_cast<Il2CppClass*>( current_state->klass ) != current_state->klass->_1.klass )
		return;

	game::main_camera = std::uintptr_t( 0 );
	auto gameobjectmanager = *reinterpret_cast<game_object_manager**>( game::unity + signatures::game_object_manager );
	for ( auto i = gameobjectmanager->tagged_objects; std::uintptr_t( i ) != std::uintptr_t( &gameobjectmanager->last_tagged_object ); i = i->next_node ) {
		auto gameobject = i->object;
		if ( gameobject->get_object_tag( ) == 5 ) {
			auto component_size = *reinterpret_cast<int32_t*>( std::uintptr_t( gameobject ) + unity::components_size );
			auto components_ptr = *reinterpret_cast<uintptr_t*>( std::uintptr_t( gameobject ) + unity::components_ptr );

			for ( auto compoment_index = 0; compoment_index < component_size; compoment_index++ ) {
				auto current_compoment = *reinterpret_cast<uintptr_t*>( components_ptr + ( compoment_index * 0x10 ) + 0x8 );
				if ( !current_compoment )
					continue;

				auto compoment_mono = *reinterpret_cast<Il2CppObject**>( current_compoment + unity::mono_ptr );
				if ( !compoment_mono || reinterpret_cast<Il2CppClass*>( compoment_mono->klass ) != compoment_mono->klass->_1.klass )
					continue;

				const constexpr auto Camera = fnv::hash_constexpr( "Camera" );
				if ( fnv::hash_runtime( compoment_mono->klass->_1.name ) != Camera )
					continue;

				if ( *reinterpret_cast<bool*>( current_compoment + unity::camera::enabled ) ) {
					game::main_camera = current_compoment;
					break;
				}
			}
			if ( game::main_camera )
				break;
		}
	}

	if ( !game::main_camera )
		return;

	/*patched
	if ( settings::cheat::make_me_reach ) {
		auto catapult_services = *reinterpret_cast<FGClient_CatapultServices_CatapultServices_c**>( game::game + signatures::catapult_services );
		if ( !catapult_services )
			return;

		auto catapult_services_instance = catapult_services->static_fields->_Instance_k__BackingField;
		if ( !catapult_services_instance )
			return;

		auto wallet_service = reinterpret_cast<FGClient_CatapultServices_PlayerWalletService_o*>( catapult_services_instance->fields._PlayerWalletService_k__BackingField );
		if ( !wallet_service )
			return;

		wallet_service->fields._Wallet_k__BackingField->fields._Kudos_k__BackingField = 1000000;
		wallet_service->fields._Wallet_k__BackingField->fields._Crowns_k__BackingField = 10000;

		settings::cheat::make_me_reach = false;
	}*/

	auto& io = ImGui::GetIO( );

	const constexpr auto StateGameInProgress = fnv::hash_constexpr( "StateGameInProgress" );
	const constexpr auto round_jinxed = fnv::hash_constexpr( "round_jinxed" );
	const constexpr auto round_door_dash = fnv::hash_constexpr( "round_door_dash" );
	const constexpr auto round_tip_toe = fnv::hash_constexpr( "round_tip_toe" );
	const constexpr auto round_match_fall = fnv::hash_constexpr( "round_match_fall" );

	if ( fnv::hash_runtime( current_state->klass->_1.name ) == StateGameInProgress ) {
		auto state_game_in_progress = reinterpret_cast<FGClient_StateGameInProgress_o*>( current_state );
		auto client_game_manager = state_game_in_progress->fields._clientGameManager;
		if ( !client_game_manager )
			return;
		auto player_list = client_game_manager->fields._players;
		if ( !player_list )
			return;

		auto game_state_view = instance->fields._GameStateView_k__BackingField;
		if ( !game_state_view )
			return;

		auto current_game_level = game_state_view->fields.CurrentGameLevelName;
		if ( !current_game_level )
			return;

		auto game_level = fnv::whash_runtime( current_game_level->fields.c_str );

		auto get_character_team_id = [ & ] ( uint32_t net_id ) -> int32_t {
			for ( auto i = 0; i < client_game_manager->fields._playerTeamManager->fields._allTeams->fields._size; i++ ) {
				auto team = client_game_manager->fields._playerTeamManager->fields._allTeams->fields._items->m_Items[ i ];
				if ( !team )
					continue;

				for ( auto y = 0; y < team->fields._members->fields._size; y++ ) {
					auto player_data = team->fields._members->fields._items->m_Items[ y ];
					if ( !player_data )
						continue;

					if ( player_data->fields.objectNetID.fields.m_NetworkID != net_id )
						continue;

					return player_data->fields.TeamID;
				}
			}
			return -1;
		};

		auto my_player_team_id = get_character_team_id( client_game_manager->fields._myPlayerNetID.fields.m_NetworkID );

		for ( auto i = 0; i < player_list->fields.count; i++ ) {
			auto character = reinterpret_cast<FallGuysCharacterController_o*>( player_list->fields.entries->m_Items[ i ].fields.value );
			if ( !character )
				continue;

			if ( character->fields._IsLocalPlayer_k__BackingField ) {
				auto game_object = *reinterpret_cast<uintptr_t*> ( *reinterpret_cast<uintptr_t*> ( std::uintptr_t( character ) + unity::native_ptr ) + unity::compoment_owner );
				auto character_transform = *reinterpret_cast<uintptr_t*> ( *reinterpret_cast<uintptr_t*> ( game_object + unity::components_ptr ) + unity::transform_compoment );
				
				if (settings::cheat::super_grab_enabled) {
					character->fields._data->fields.playerGrabDetectRadius = FLT_MAX;
					character->fields._data->fields.playerGrabCheckDistance = FLT_MAX;
					character->fields._data->fields.playerGrabberMaxForce = FLT_MAX;
					character->fields._data->fields.playerGrabBreakTime = FLT_MAX;
					character->fields._data->fields.armLength = FLT_MAX;
					character->fields._data->fields.playerGrabCheckPredictionBase = FLT_MAX;
					character->fields._data->fields.playerGrabMaxHeightDifference = FLT_MAX;
					character->fields._data->fields.playerGrabbeeMaxForce = 0;
					character->fields._data->fields.playerGrabImmediateVelocityReduction = 1;
					character->fields._data->fields.playerGrabberDragDirectionContribution = 1;
					character->fields._data->fields.grabCooldown = 0;
					character->fields._data->fields.playerGrabRegrabDelay = 0;
					character->fields._data->fields.playerGrabBreakTimeJumpInfluence = 0;
					character->fields._data->fields.forceReleaseRegrabCooldown = 0;
					character->fields._data->fields.breakGrabAngle = 360;
					character->fields._data->fields.playerGrabBreakSeparationForce = 0.f;
					character->fields._data->fields.playerGrabbeeInvulnerabilityWindow = 0.f;
				} else {
					character->fields._data->fields.playerGrabDetectRadius = default_playerGrabDetectRadius;
					character->fields._data->fields.playerGrabCheckDistance = default_playerGrabCheckDistance;
					character->fields._data->fields.playerGrabberMaxForce = default_playerGrabberMaxForce;
					character->fields._data->fields.playerGrabBreakTime = default_playerGrabBreakTime;
					character->fields._data->fields.armLength = default_armLength;
					character->fields._data->fields.playerGrabCheckPredictionBase = default_playerGrabCheckPredictionBase;
					character->fields._data->fields.playerGrabImmediateVelocityReduction = default_playerGrabImmediateVelocityReduction;
					character->fields._data->fields.playerGrabberDragDirectionContribution = default_playerGrabberDragDirectionContribution;
					character->fields._data->fields.grabCooldown = default_grabCooldown;
					character->fields._data->fields.playerGrabRegrabDelay = default_playerGrabRegrabDelay;
					character->fields._data->fields.playerGrabBreakTimeJumpInfluence = default_playerGrabBreakTimeJumpInfluence;
					character->fields._data->fields.forceReleaseRegrabCooldown = default_forceReleaseRegrabCooldown;
					character->fields._data->fields.breakGrabAngle = default_breakGrabAngle;
					character->fields._data->fields.playerGrabbeeMaxForce = default_playerGrabbeeMaxForce;
					character->fields._data->fields.playerGrabBreakSeparationForce = default_playerGrabBreakSeparationForce;
					character->fields._data->fields.playerGrabbeeInvulnerabilityWindow = default_playerGrabbeeInvulnerabilityWindow;
				}

				if ( settings::movement::fly_enabled ) {
					character->fields._ApplyGravity_k__BackingField = false;
					character->fields._data->fields.carryMaxSpeed = settings::movement::fly_speed;
					auto camera_transform = *reinterpret_cast<uintptr_t*> ( *reinterpret_cast<uintptr_t*>( *reinterpret_cast<uintptr_t*> ( std::uintptr_t( game::main_camera ) + unity::compoment_owner ) + unity::components_ptr ) + unity::transform_compoment );
					auto direction = get_forward( camera_transform );

					auto movement = *reinterpret_cast<uintptr_t*> ( *reinterpret_cast<uintptr_t*> ( std::uintptr_t( character->fields._rigidbody ) + unity::native_ptr ) + 0x60 );
					auto veloctiy = reinterpret_cast<vector*> ( movement + 0x15C );

					if ( io.KeysDown[ 0x57 ] || io.NavInputs[ ImGuiNavInput_LStickUp ] > 0.f )
						*veloctiy = direction * settings::movement::fly_speed;
					else
						*veloctiy = vector( 0, 0, 0 );

					if ( io.KeysDown[ VK_SPACE ] || io.NavInputs[ ImGuiNavInput_Activate ] > 0.f )
						veloctiy->z = settings::movement::fly_speed;
					else if ( io.KeysDown[ VK_SHIFT ] || io.NavInputs[ ImGuiNavInput_Cancel ] > 0.f )
						veloctiy->z = -settings::movement::fly_speed;
					else
						veloctiy->z = 0.f;

				} else if ( cheat_helper::disable_fly ) {
					character->fields._data->fields.carryMaxSpeed = default_carryMaxSpeed;
					character->fields._ApplyGravity_k__BackingField = true;
					cheat_helper::disable_fly = false;
				}

				character->fields._data->fields.anyCollisionStunForce = settings::movement::disable_stun_collision ? FLT_MAX : default_anyCollisionStunForce;
				character->fields._data->fields.dynamicCollisionStunForce = settings::movement::disable_stun_collision ? FLT_MAX : default_dynamicCollisionStunForce;

				character->fields._ragdollController->fields.CollisionThreshold = settings::movement::disable_object_collisions ? FLT_MAX : default_CollisionThreshold;
				if ( settings::movement::disable_object_collisions )
					character->fields._ragdollController->fields.CollisionUpperBodyTransfer = 0.f;

				if ( settings::movement::speed_enabled ) {
					character->fields._data->fields.carryMaxSpeed = settings::movement::speed_boost;
					character->fields._data->fields.normalMaxSpeed = settings::movement::speed_boost;

				} else if ( cheat_helper::disable_speed ) {
					character->fields._data->fields.carryMaxSpeed = default_carryMaxSpeed;
					character->fields._data->fields.normalMaxSpeed = default_max_speed;
					cheat_helper::disable_speed = false;
				}
			} else {
				if ( game_level == round_jinxed ) {
					if ( get_character_team_id( (uint32_t)player_list->fields.entries->m_Items[ i ].fields.key ) != my_player_team_id ) {
						if ( !character->fields._ActiveTagAccessory_k__BackingField
							|| std::uintptr_t( character->fields._ActiveTagAccessory_k__BackingField ) != std::uintptr_t( character->fields._infectedAccessory )
							|| !character->fields._ActiveTagAccessory_k__BackingField->fields._isAccessoryEnabled ) {
							vector vec_min, vec_max;
							if ( get_bounding_box2d( character->fields._collider, vec_min, vec_max ) )
								draw_manager::add_rect_on_screen( vec_min, vec_max, ImColor( 1.f, 0.f, 0.f ), 0.f, -1, 5.f );
						}
					}
				}
				if (settings::cheat::player_esp_enabled) {
					vector vec_min, vec_max;
					if (get_bounding_box2d(character->fields._collider, vec_min, vec_max))
						draw_manager::add_rect_on_screen(vec_min, vec_max, ImColor(0.f, 1.f, 0.f), 0.f, -1, 2.f);
				}
			}
		}

		if ( game_level == round_door_dash ||
			game_level == round_tip_toe ||
			game_level == round_match_fall ) {
			auto gameobjectmanager = *reinterpret_cast<game_object_manager**>( game::unity + signatures::game_object_manager );
			for ( auto i = gameobjectmanager->active_objects; std::uintptr_t( i ) != std::uintptr_t( &gameobjectmanager->last_active_object ); i = i->next_node ) {	
			
				if ( !i )
					break;

				auto current_object = i->object;
				if ( current_object ) {
					auto component_size = *reinterpret_cast<int32_t*>( std::uintptr_t( current_object ) + unity::components_size );
					auto components_ptr = *reinterpret_cast<uintptr_t*>( std::uintptr_t( current_object ) + unity::components_ptr );

					if ( component_size > 1 && components_ptr ) {
						RealDoorController_o* real_door = nullptr;
						TipToe_Platform_o* tiptoe_platform = nullptr;
						MatchFallManager_o* match_fall_manager = nullptr;

						for ( auto compoment_index = 0; compoment_index < component_size; compoment_index++ ) {
							auto current_compoment = *reinterpret_cast<uintptr_t*>( components_ptr + ( compoment_index * 0x10 ) + 0x8 );
							if ( !current_compoment )
								continue;
							auto compoment_mono = *reinterpret_cast<Il2CppObject**>( current_compoment + unity::mono_ptr );
							if ( !compoment_mono || reinterpret_cast<Il2CppClass*>( compoment_mono->klass ) != compoment_mono->klass->_1.klass )
								continue;

							auto class_name = fnv::hash_runtime( compoment_mono->klass->_1.name );

							switch ( class_name ) {
								case fnv::hash_constexpr( "RealDoorController" ):
									real_door = reinterpret_cast<RealDoorController_o*>( compoment_mono );
									break;
								case fnv::hash_constexpr( "TipToe_Platform" ):
									tiptoe_platform = reinterpret_cast<TipToe_Platform_o*>( compoment_mono );
									break;
								case fnv::hash_constexpr( "MatchFallManager" ):
									match_fall_manager = reinterpret_cast<MatchFallManager_o*>( compoment_mono );
									break;
							}
						}

						if ( real_door ) {
							if ( real_door->fields._hasBroken )
								continue;

							vector vec_min, vec_max;
							if ( get_bounding_box2d( real_door->fields._wallCollider, vec_min, vec_max ) )
								draw_manager::add_rect_on_screen( vec_min, vec_max, ImColor( 0.f, 0.f, 1.f ), 0.f, -1, 3.f );
						}

						if ( tiptoe_platform ) {
							if ( tiptoe_platform->fields._isFakePlatform )
								continue;

							auto character_transform = *reinterpret_cast<uintptr_t*> ( *reinterpret_cast<uintptr_t*> ( std::uintptr_t( current_object ) + unity::components_ptr ) + unity::transform_compoment );

							if ( !character_transform )
								continue;

							vector world = get_position( character_transform );
							vector screen;
							if ( world_to_screen( world, screen ) )
								draw_manager::add_filled_rect_on_screen( screen, screen + vector( 15, 15 ), ImColor( 1.f, 0.f, 0.f ) );
						}

						if ( match_fall_manager ) {
							for ( auto j = 0; j < match_fall_manager->fields.Tiles->fields._size; j++ ) {
								auto tile = match_fall_manager->fields.Tiles->fields._items->m_Items[ j ];
								if ( !tile )
									continue;

								if ( tile->fields._tileSurfaceOnObject )
									reinterpret_cast<void( __stdcall* )( void*, bool, void* )>( game::unity + signatures::game_object_custom_set_active )( tile->fields._tileSurfaceOnObject, true, 0 );

								if ( tile->fields._tileSurfaceOffObject )
									reinterpret_cast<void( __stdcall* )( void*, bool, void* )>( game::unity + signatures::game_object_custom_set_active )( tile->fields._tileSurfaceOffObject, false, 0 );
							}
						}
					}
				}
			}
		}
	}

	/*
	if ( GetAsyncKeyState( VK_F1 ) ) {
		printf( "MAP: %ws\n", instance->fields._GameStateView_k__BackingField->fields.CurrentGameLevelName->fields.c_str );
		auto gameobjectmanager = *reinterpret_cast<game_object_manager**>( game::unity + signatures::game_object_manager );
		for ( auto i = gameobjectmanager->active_objects; std::uintptr_t( i ) != std::uintptr_t( &gameobjectmanager->last_active_object ); i = i->next_node ) {
			auto current_object = i->object;
			if ( current_object ) {
				auto component_size = *reinterpret_cast<int32_t*>( std::uintptr_t( current_object ) + unity::components_size );
				auto components_ptr = *reinterpret_cast<uintptr_t*>( std::uintptr_t( current_object ) + unity::components_ptr );
				if ( component_size > 1 && components_ptr ) {
					for ( auto compoment_index = 0; compoment_index < component_size; compoment_index++ ) {
						auto current_compoment = *reinterpret_cast<uintptr_t*>( components_ptr + ( compoment_index * 0x10 ) + 0x8 );
						if ( !current_compoment )
							continue;
						auto compoment_mono = *reinterpret_cast<Il2CppObject**>( current_compoment + unity::mono_ptr );
						if ( !compoment_mono || reinterpret_cast<Il2CppClass*>( compoment_mono->klass ) != compoment_mono->klass->_1.klass )
							continue;

						auto class_name = fnv::hash_runtime( compoment_mono->klass->_1.name );
					}
				}
				auto transform = *reinterpret_cast<uintptr_t*>( std::uintptr_t( current_object ) + unity::components_ptr );
				if ( transform ) {
					transform = *reinterpret_cast<uintptr_t*>( transform + unity::transform_compoment );
					if ( transform ) {
						auto transform_mono = *reinterpret_cast<UnityEngine_Transform_o**>( transform + unity::mono_ptr );
						if ( transform_mono ) {
							if ( !strstr( transform_mono->klass->_1.name, "Rect" ) ) {
								auto position = get_position( transform );
								auto screen = vector( );
								if ( world_to_screen( position, screen ) ) {
									draw_manager::add_text_on_screen( screen, ImColor( 0.f, 0.f, 0.f ), 19, "%s", current_object->get_name( ) );
								}
							}
						}
					}
				}
			}
		}
	}*/
}

ID3D11RenderTargetView* d3d_render_target_view = NULL;
struct dxgi_present {
	static long __stdcall hooked( IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags ) {
		std::call_once( init_device, [ & ] ( ) {
			ImGui::CreateContext( );
			p_swap_chain->GetDevice( __uuidof( d3d11_device ), reinterpret_cast<void**>( &( d3d11_device ) ) );
			d3d11_device->GetImmediateContext( &d3d11_device_context );

			ID3D11Texture2D* renderTargetTexture = nullptr;
			if ( SUCCEEDED( p_swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<LPVOID*>( &renderTargetTexture ) ) ) ) {
				D3D11_RENDER_TARGET_VIEW_DESC desc = {};
				memset( &desc, 0, sizeof( desc ) );
				desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				d3d11_device->CreateRenderTargetView( renderTargetTexture, &desc, &d3d_render_target_view );
				renderTargetTexture->Release( );
			}

			ImGui::GetIO( ).ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			ImGui::GetIO( ).ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
			ImGui::GetIO( ).ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			ImGui_ImplWin32_Init( hwnd );
			ImGui_ImplDX11_Init( d3d11_device, d3d11_device_context );
			ImGui_ImplDX11_CreateDeviceObjects( );
			ImGui::StyleColorsDark( );
			} );

		ImGui_ImplDX11_NewFrame( );
		ImGui_ImplWin32_NewFrame( );
		ImGui::NewFrame( );

		draw_manager::begin_rendering( );

		update( );

		menu::update_indicators( );

		draw_manager::end_rendering( );

		if ( render::menu_is_open ) {
			menu::draw( );
		}

		menu::update_keys( );

		ImGui::Render( );
		d3d11_device_context->OMSetRenderTargets( 1, &d3d_render_target_view, NULL );
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
		return m_original( p_swap_chain, sync_interval, flags );
	}

	static decltype( &hooked ) m_original;
};
decltype( dxgi_present::m_original ) dxgi_present::m_original;

WNDPROC original_wndproc;
LRESULT wnd_proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	if ( ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) )
		return true;

	if ( render::menu_is_open )
		return true;

	return CallWindowProcW( original_wndproc, hWnd, uMsg, wParam, lParam );
}

void render::load( ) {
	auto unity_player = std::uintptr_t( GetModuleHandle( "UnityPlayer.dll" ) );
	hwnd = FindWindow( nullptr, "FallGuys_client" );
	original_wndproc = reinterpret_cast<WNDPROC>( SetWindowLongPtrW( hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( wnd_proc ) ) );

	swap_chain = reinterpret_cast<IDXGISwapChain * ( __stdcall* )( )>( unity_player + signatures::swap_chain )( );
	swap_chain_vmt = std::make_unique<::vmt_smart_hook>( swap_chain );
	swap_chain_vmt->apply_hook<dxgi_present>( 8 );
}

void render::unload( ) {
	SetWindowLongPtrW( hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( original_wndproc ) );
	swap_chain_vmt->unhook( );
}
