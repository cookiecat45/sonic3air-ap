/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once
#include "oxygen_netcore/network/Sockets.h"
#include "sonic3air/client/archipelago/apclientpp/apclient.hpp"
#include <lemon/program/StringRef.h>

class ArchipelagoClient
{
public:
	bool startConnection();
	void setupHandlers();
	void stopConnection();
	bool isConnected();
	void updateConnection(float timeElapsed);
	void sendLocation(uint64 id);

private:
	bool mConnecting = false;

public:
	std::unique_ptr<APClient> mClient;
};
