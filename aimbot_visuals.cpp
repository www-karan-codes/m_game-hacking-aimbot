#include "aimbot_visuals.hpp"
#include "aimbot.hpp"
#include "../ImGui/imgui.h"
#include "../GameFunctions/game_functions.hpp"

namespace gamehacking::aimbot::visuals
{
	void Draw(void)
	{
		if (!aimbot::enabled)
		{
			return;
		}

		ImGui::SetNextWindowPos({ 0, 0 });
		ImGui::SetNextWindowSize(gamehacking::game_functions::GetResolution());
		ImGui::Begin("aim_assist", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);

		auto imgui_draw_list = ImGui::GetWindowDrawList();

		auto& aimbot_information_data = gamehacking::aimbot::aimbot_information_data;

		for (auto& aimbot_information : aimbot_information_data)
		{
			imgui_draw_list->AddCircleFilled(aimbot_information.prediction_projection_, 3, ImColor(255, 0, 255));
		}

		ImGui::End();
	}
}