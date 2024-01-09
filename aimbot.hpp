#pragma once

#include <list>
#include "../BaseStructures/base_structures.hpp"


namespace gamehacking::aimbot
{
	struct AimbotInformation
	{
		gamehacking::math::Vector3D prediction_projection_;
		int team_;
	};

	extern std::list<AimbotInformation> aimbot_information_data;

	//extern gamehacking::math::Vector3D screen_center;
	enum AimbotMode { kClosestDistance, kClosestXhair };
	extern const char* mode_labels[];
	extern bool enabled;
	extern bool aimbot_enabled;
	extern gamehacking::base_structures::Player target_player;

	namespace settings
	{
		extern AimbotMode aimbot_mode;
		extern bool stay_locked_to_target;
		extern bool auto_lock_to_new_target;
		extern bool friendly_fire;
		extern bool need_line_of_sight;
		extern bool target_everyone;

		extern unsigned int maximum_iterations;
		extern double epsilon;
	}

	void Tick(void);
	void Enable(void);
	void Reset(void);
	void Disable(void);
	bool FindTarget(void);
}