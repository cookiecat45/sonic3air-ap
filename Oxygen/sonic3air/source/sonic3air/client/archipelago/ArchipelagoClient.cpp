/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include <sys/socket.h>
#include <nlohmann/json.hpp>
#include "sonic3air/pch.h"
#include "sonic3air/client/archipelago/ArchipelagoClient.h"
#include "oxygen/application/Application.h"
#include "oxygen/helper/JsonHelper.h"
#include "oxygen/simulation/CodeExec.h"
#include "oxygen/simulation/LogDisplay.h"
#include "oxygen/simulation/Simulation.h"
#include <imgui.h>
#include <imgui_stdlib.h>

using json = nlohmann::json;
static char serverAddress[512] = "localhost:38281";
static char slotName[512] = "";
static char password[512] = "";
static std::string errorMessage = "";

bool ArchipelagoClient::startConnection()
{
	return false;
	/*
	if (mConnecting || isConnected())
	{
		return false;
	}

	mConnecting = true;
	mClient.reset();
	printf("Connecting to AP...\n");
	mClient.reset(new APClient("", "Sonic 3 A.I.R.", "localhost:38281"));
	setupHandlers();
	return true;
	*/
}

void ArchipelagoClient::setupHandlers()
{
	mClient->set_room_info_handler([this](){
        mClient->ConnectSlot(slotName, password, 7);
    });
	mClient->set_slot_connected_handler([this](const json&){
        CodeExec& codeExec = Application::instance().getSimulation().getCodeExec();
		codeExec.mForceDisableExec = false;
		mConnecting = false;
    });
}

void ArchipelagoClient::stopConnection()
{
	mClient.reset();
}

bool ArchipelagoClient::isConnected()
{
	return mClient && mClient->get_state() == APClient::State::SLOT_CONNECTED;
}

void ArchipelagoClient::updateConnection(float timeElapsed)
{
	CodeExec& codeExec = Application::instance().getSimulation().getCodeExec();
	if (!mClient || codeExec.mForceDisableExec)
	{
		ImGui::Begin("Connection Input");
		codeExec.mForceDisableExec = true;
		ImGui::InputText("Server address", serverAddress, sizeof(serverAddress), ImGuiInputTextFlags_CharsNoBlank);
		ImGui::InputText("Slot name", slotName, sizeof(slotName));
		ImGui::InputText("Password", password, sizeof(password));
		bool connectClicked = ImGui::Button(mConnecting ? "Connecting..." : "Connect");
		if (connectClicked && !mConnecting)
		{
			if (std::strlen(serverAddress) <= 0)
			{
				errorMessage = "Please enter a server address";
				ImGui::OpenPopup("Input Error");
			}
			else if (std::strlen(slotName) <= 0)
			{
				errorMessage = "Please enter a slot name";
				ImGui::OpenPopup("Input Error");
			}
			else
			{
				mConnecting = true;
				mClient.reset();
				printf("Connecting to AP...\n");
				mClient.reset(new APClient("", "Sonic 3 A.I.R.", serverAddress));
				setupHandlers();
			}
		}
		
		if (ImGui::BeginPopupModal("Input Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
		{
			ImGui::Text("%s", errorMessage.c_str());
			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) 
			{
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::EndPopup();
		}
		
		ImGui::End();
		return;
	}
	
	mClient->poll();
	APClient::State state = mClient->get_state();
	mConnecting = (state > APClient::State::DISCONNECTED && state < APClient::State::SLOT_CONNECTED);
}

void ArchipelagoClient::sendLocation(uint64 id)
{
	std::list<int64_t> idList;
	idList.push_front(id);
	mClient->LocationChecks(idList);
}
