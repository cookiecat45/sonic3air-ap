/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#ifdef PLATFORM_LINUX
#include <sys/socket.h>
#endif
#include <nlohmann/json.hpp>
#include "sonic3air/pch.h"
#include "sonic3air/client/archipelago/ArchipelagoClient.h"
#include "oxygen/application/Application.h"
#include "oxygen/helper/JsonHelper.h"
#include "oxygen/simulation/Simulation.h"
#include "oxygen/simulation/CodeExec.h"
#include "oxygen/simulation/LogDisplay.h"
#include <imgui.h>

using json = nlohmann::json;
static char serverAddress[512] = "localhost:38281";
static char slotName[512] = "";
static char password[512] = "";
static std::string errorMessage = "";
static std::string socketError = "";

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
        Simulation& sim = Application::instance().getSimulation();
		sim.mDisableInput = false;
		mConnecting = false;
		socketError = "";
		ArchipelagoClient::callScriptFunction("Archipelago.OnConnected");
    });
	mClient->set_socket_error_handler([this](const std::string& msg) {
		if (msg != "TLS handshake failed") // don't immediately fail if this is the case - this always happens on localhost
		{
			socketError = msg;
		}
	});
	mClient->set_slot_refused_handler([this](const std::list<std::string>& msg) {
		socketError = "Rejected by Archipelago server.\nMost likely an incorrect slot name, password, or port";
	});
	mClient->set_socket_disconnected_handler([this](){
        if (!mConnecting)
		{
			ArchipelagoClient::callScriptFunction("Archipelago.OnDisconnected");
		}
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
	Simulation& sim = Application::instance().getSimulation();
	if (!mClient || sim.mDisableInput)
	{
		if (!ImGui::GetCurrentContext())
			return;

		//sim.mDisableInput = true;
		ImGui::Begin("Connection Input");
		ImGui::InputText("Server address", serverAddress, sizeof(serverAddress), ImGuiInputTextFlags_CharsNoBlank);
		ImGui::InputText("Slot name", slotName, sizeof(slotName));
		ImGui::InputText("Password", password, sizeof(password));
		bool connectClicked = ImGui::Button(mConnecting ? "Connecting..." : "Connect");
		if (connectClicked && !mConnecting)
		{
			if (std::strlen(serverAddress) <= 0)
			{
				errorMessage = "Please enter a server address";
				ImGui::OpenPopup("Error");
			}
			else if (std::strlen(slotName) <= 0)
			{
				errorMessage = "Please enter a slot name";
				ImGui::OpenPopup("Error");
			}
			else
			{
				mConnecting = true;
				mLastConnect = now();
				mClient.reset();
				printf("Connecting to AP...\n");
				mClient.reset(new APClient("", "Sonic 3 A.I.R.", serverAddress));
				setupHandlers();
			}
		}
		else if (mConnecting && mClient)
		{
			mClient->poll();
			bool timeOut = static_cast<unsigned long>(now() - mLastConnect) > 11000;
			if (timeOut)
			{
				socketError = "Connection timed out";
			}
			
			if (socketError.length() > 0)
			{
				mConnecting = false;
				mClient.reset();
				errorMessage = "Connection failed: " + socketError;
				socketError = "";
				ImGui::OpenPopup("Error");
			}
		}
		
		if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
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

void ArchipelagoClient::callScriptFunction(lemon::FlyweightString functionName)
{
	Application::instance().getSimulation().getCodeExec().getLemonScriptRuntime().callFunctionByName(functionName);
}

void ArchipelagoClient::sendLocation(uint64 id)
{
	std::list<int64_t> idList;
	idList.push_front(id);
	mClient->LocationChecks(idList);
}
