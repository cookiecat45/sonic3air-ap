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
	void callScriptFunction(lemon::FlyweightString functionName);

private:
	bool mConnecting = false;
	unsigned long mLastConnect = 0;
	static unsigned long now()
    {
		#if defined WIN32 || defined _WIN32
		#if WINVER >= 0x0600 || _WIN32_WINNT >= 0x0600
		if (sizeof(unsigned long) > 4) {
			return static_cast<unsigned long>(GetTickCount64());
		}
		#endif
		return static_cast<unsigned long>(GetTickCount());
		#else
		timespec ts{};
		clock_gettime(CLOCK_MONOTONIC, &ts);
		auto ms = static_cast<unsigned long>(
			static_cast<uint64_t>(ts.tv_sec) * 1000);
		ms += static_cast<unsigned long>(ts.tv_nsec / 1000000);
		return ms;
		#endif
    }

public:
	std::unique_ptr<APClient> mClient;
};
