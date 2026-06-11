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

using json = nlohmann::json;

bool ArchipelagoClient::startConnection()
{
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
}

void ArchipelagoClient::setupHandlers()
{
	mClient->set_room_info_handler([this](){
        mClient->ConnectSlot("Player1", "", 7);
    });
	mClient->set_slot_connected_handler([this](const json&){
        
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
	if (!mClient)
		return;

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
