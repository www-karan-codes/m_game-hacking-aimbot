#include "aimbot.hpp"
#include "../Math/math.hpp"
#include "../GameFunctions/game_functions.hpp"
#include "../BaseStructures/base_structures.hpp"

namespace gamehacking::aimbot
{
    std::list<AimbotInformation> aimbot_information_data;
    //gamehacking::math::Vector3D screen_center;
	const char* mode_labels[] = { "Closest distance", "Closest to xhair" };
	bool enabled = true;
	bool aimbot_enabled = false;
	gamehacking::base_structures::Player target_player(nullptr);

    namespace settings
    {
        AimbotMode aimbot_mode = AimbotMode::kClosestXhair;
        bool stay_locked_to_target = true;
        bool auto_lock_to_new_target = false;
        bool friendly_fire = true;
        bool need_line_of_sight = false;
        bool target_everyone = false;
        unsigned int maximum_iterations = 10;
        double epsilon = 0.05;
    }

	void Enable(void)
	{
		enabled = true;
	}

	void Reset(void)
	{
		target_player = gamehacking::base_structures::Player(nullptr);
		Enable();
	}

	void Disable(void)
	{
		Reset();
		enabled = false;
	}

    bool FindTarget(void)
    {

        auto resolution = game_functions::GetResolution();
        auto screen_center = resolution / 2;

        auto current_target_object = target_player.object_;

        auto need_to_find_target = true;  //! aimbot_settings.target_everyone;

        if (settings::stay_locked_to_target) {
            auto find_if_current_target_exists_in_players_list = false;
            for (auto& player : gamehacking::base_structures::players) {
                if (player.object_ == target_player.object_) {
                    find_if_current_target_exists_in_players_list = true;
                    break;
                }
            }

            if (!find_if_current_target_exists_in_players_list || !gamehacking::base_structures::Player::IsValid(target_player.object_)) {
                if (!settings::auto_lock_to_new_target && current_target_object != nullptr) {
                    Disable();
                    return false;
                }
            }
            else {
                need_to_find_target = false;
            }
        }

        if (need_to_find_target) {
            current_target_object = nullptr;
            auto rotation_vector = gamehacking::base_structures::my_player.forward_;
            for (auto& player : gamehacking::base_structures::players) {
                if (!player.is_valid_ || player.object_ == gamehacking::base_structures::my_player.object_ || !gamehacking::base_structures::Player::IsValid(player.object_)) {
                    continue;
                }

                auto same_team = gamehacking::base_structures::my_player.team_ == player.team_;
                auto line_of_sight = gamehacking::game_functions::InLineOfSight(player);
                if ((same_team && !settings::friendly_fire) || (!gamehacking::math::Vector3D::IsInFieldOfView(gamehacking::base_structures::my_player.location_, gamehacking::base_structures::my_player.forward_, player.location_) && settings::aimbot_mode == AimbotMode::kClosestXhair) || (!line_of_sight && settings::need_line_of_sight))
                    continue;

                switch (settings::aimbot_mode) {
                case AimbotMode::kClosestXhair: {
                    static double distance = 0;
                    auto enemy_location = player.location_;

                    /*
                    if (!game_functions::IsInHorizontalFieldOfView(player->location_, aimbot_settings.aimbot_horizontal_fov_angle))
                        continue;
                    */

                    auto projection_vector = game_functions::Project(enemy_location);
                    if (!current_target_object) {
                        current_target_object = player.object_;
                        distance = (projection_vector - screen_center).Magnitude();
                    }
                    else {
                        double current_distance = (projection_vector - screen_center).Magnitude();
                        if (current_distance < distance) {
                            current_target_object = player.object_;
                            distance = current_distance;
                        }
                    }
                } break;

                case AimbotMode::kClosestDistance: {
                    static double distance = 0;
                    if (!current_target_object) {
                        current_target_object = player.object_;
                        distance = (gamehacking::base_structures::my_player.location_ - player.location_).Magnitude();
                    }
                    else {
                        double current_distance = (gamehacking::base_structures::my_player.location_ - player.location_).Magnitude();
                        if (current_distance < distance) {
                            current_target_object = player.object_;
                            distance = current_distance;
                        }
                    }
                } break;
                }
            }

            if (!current_target_object && !settings::auto_lock_to_new_target) {
                // Disable();
                // return false;
            }
        }

        target_player = gamehacking::base_structures::Player(current_target_object);

            // return true;
        return current_target_object != nullptr;
    }

    void Tick(void)
    {
        aimbot_information_data.clear();

        if (!settings::target_everyone) {
            bool triggerbot_success = false;
            if (enabled && FindTarget()) {

                gamehacking::math::Vector3D prediction;
                auto result = gamehacking::base_structures::my_player.PredictAim(target_player, prediction);

                if (result) {
                    auto projection = game_functions::Project(prediction);
                    double height = -1;
                    double width = -1;

                    /*
                    if ((imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds || imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kFilledBounds) && height == -1) {
                        FVector2D center_projection = game_functions::Project(target_player.location_);
                        target_player.location_.Z += target_player.character_->CylinderComponent->CollisionHeight;  // this is HALF the height in reality
                        FVector2D head_projection = game_functions::Project(target_player.location_);
                        target_player.location_.Z -= target_player.character_->CylinderComponent->CollisionHeight;  // this is HALF the height in reality
                        height = abs(head_projection.Y - center_projection.Y);
                        width = height * (target_player.character_->CylinderComponent->CollisionRadius / target_player.character_->CylinderComponent->CollisionHeight);
                    }
                    */

                    if (gamehacking::math::Vector3D::IsInFieldOfView(gamehacking::base_structures::my_player.location_, gamehacking::base_structures::my_player.forward_, prediction)) {
                        AimbotInformation aimbot_information;
                        aimbot_information.prediction_projection_ = projection;
                        aimbot_information.team_ = target_player.team_;
                        aimbot_information_data.push_back(aimbot_information);
                    }

                    if (aimbot_enabled)
                    {
                        prediction.z_ -= target_player.collision_height_;
                        gamehacking::game_functions::AimAt(prediction);
                    }

                    /*
                    if (aimbot_settings.auto_aim || aimbot_enabled) {
                        if (aimbot_settings.aimbot_target_center_of_body)
                            prediction.Z -= target_player.character_->CylinderComponent->CollisionHeight / 1;  // Don't divide by 2?
                        FRotator aim_rotator = math::VectorToRotator(prediction - game_data::my_player.location_);
                        FRotator& aim_rotator_reference = aim_rotator;
                        game_data::local_player_controller->SetRotation(aim_rotator_reference);
                    }
                    */
                }
            }
            else {
                /*
                if (aimbot_settings.use_triggerbot && !triggerbot_success) {
                    game_data::local_player_controller->StopFire(fire_mode);
                }
                */
            }
        }
        else {
            for (auto& player : gamehacking::base_structures::players)
            {
                if (!player.is_valid_ || player.object_ == gamehacking::base_structures::my_player.object_ || !gamehacking::base_structures::Player::IsValid(player.object_)) {
                    continue;
                }

                auto same_team = gamehacking::base_structures::my_player.team_ == player.team_;
                auto line_of_sight = gamehacking::game_functions::InLineOfSight(player);
                if ((same_team && !settings::friendly_fire) || (!line_of_sight && settings::need_line_of_sight))
                    continue;

                gamehacking::math::Vector3D prediction;
                auto result = gamehacking::base_structures::my_player.PredictAim(player, prediction);

                if (result) {
                    auto projection = game_functions::Project(prediction);
                    double height = -1;
                    double width = -1;

                    if (gamehacking::math::Vector3D::IsInFieldOfView(gamehacking::base_structures::my_player.location_, gamehacking::base_structures::my_player.forward_, prediction)) {
                        AimbotInformation aimbot_information;
                        aimbot_information.prediction_projection_ = projection;
                        aimbot_information.team_ = target_player.team_;
                        aimbot_information_data.push_back(aimbot_information);
                    }

                }
            }

            /*
            bool triggerbot_success = false;
            for (vector<game_data::information::Player>::iterator player = game_data::game_data.players.begin(); player != game_data::game_data.players.end(); player++) {
                if (!player->is_valid_ || player->character_ == game_data::my_player.character_) {
                    continue;
                }

                game_data::information::Player* p = (game_data::information::Player*)&*player;
                bool same_team = game_data::my_player.IsSameTeam(p);
                // bool same_team = game_functions::IsSameTeam(game_data::local_player_character, player->character_);
                bool line_of_sight = game_functions::InLineOfSight(player->character_);

                if ((same_team && !aimbot_settings.friendly_fire) || (!game_functions::IsInFieldOfView(player->location_) && aimbot_settings.aimbot_mode == AimbotMode::kClosestXhair) || (!line_of_sight && aimbot_settings.need_line_of_sight))
                    continue;

                if (!game_functions::IsInHorizontalFieldOfView(player->location_, aimbot_settings.aimbot_horizontal_fov_angle))
                    continue;

                player_world_object.SetLocation(player->location_);
                player_world_object.SetVelocity(player->velocity_);
                player_world_object.character_ = player->character_;

                bool result = abstraction::my_weapon_object.PredictAimAtTarget(&player_world_object, &prediction, muzzle_offset);

                if (result) {
                    static const bool use_trace = true;
                    if (use_trace) {
                        static FVector hit_location;
                        static FVector hit_normal;
                        static FTraceHitInfo hit_info;
                        AActor* hitActor = game_data::local_player_controller->Trace(prediction, player->location_, false, FVector(), 0, &hit_location, &hit_normal, &hit_info);

                        if (hitActor) {
                            prediction = hit_location;
                            if (player->location_.Z < hit_location.Z) {
                                prediction.Z -= player->character_->CylinderComponent->CollisionHeight;
                            }
                            else {
                                prediction.Z += player->character_->CylinderComponent->CollisionHeight;
                            }
                        }
                    }

                    FVector2D projection = game_functions::Project(prediction);
                    float height = -1;
                    float width = -1;

                    if (aimbot_settings.use_triggerbot) {
                        if (projection.X > 0 && projection.Y > 0) {
                            bool los = game_functions::InLineOfSight(player->character_);
                            if (los) {
                                FVector2D center_projection = game_functions::Project(player->location_);
                                player->location_.Z += player->character_->CylinderComponent->CollisionHeight;  // this is HALF the height in reality
                                FVector2D head_projection = game_functions::Project(player->location_);
                                player->location_.Z -= player->character_->CylinderComponent->CollisionHeight;  // this is HALF the height in reality
                                height = abs(head_projection.Y - center_projection.Y);
                                width = height * (player->character_->CylinderComponent->CollisionRadius / player->character_->CylinderComponent->CollisionHeight);

                                if (abs(game_data::screen_center.X - projection.X) < width && abs(game_data::screen_center.Y - projection.Y) < height) {
                                    if (game_data::my_player.weapon_ == game_data::Weapon::found) {
                                        // other::SendLeftMouseClick();
                                        game_data::my_player.character_->Weapon->StartFire(fire_mode);
                                        triggerbot_success = true;
                                    }
                                }
                            }
                        }
                    }

                    if ((imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds || imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kFilledBounds) && height == -1) {
                        FVector2D center_projection = game_functions::Project(player->location_);
                        player->location_.Z += player->character_->CylinderComponent->CollisionHeight;  // this is HALF the height in reality
                        FVector2D head_projection = game_functions::Project(player->location_);
                        player->location_.Z -= player->character_->CylinderComponent->CollisionHeight;  // this is HALF the height in reality
                        height = abs(head_projection.Y - center_projection.Y);
                        width = height * (player->character_->CylinderComponent->CollisionRadius / player->character_->CylinderComponent->CollisionHeight);
                    }

                    if (projection.Y >= 0) {
                        projections_of_predictions.push_back(projection);
                        aimbot_information.push_back({ (prediction - game_data::my_player.location_).Magnitude(), projection, height, width });
                    }

                    // math::PrintVector(prediction, "Prediction");
                }
            }

            if (aimbot_settings.use_triggerbot && !triggerbot_success) {
                game_data::local_player_controller->StopFire(fire_mode);
            */
        }

    }

}