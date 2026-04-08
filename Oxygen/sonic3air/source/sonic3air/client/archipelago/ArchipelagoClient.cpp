/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include <sys/socket.h>
#include "sonic3air/pch.h"
#include "sonic3air/client/archipelago/ArchipelagoClient.h"
#include "sonic3air/client/archipelago/apclientpp/apclient.hpp"

#include "oxygen/application/Application.h"
#include "oxygen/helper/JsonHelper.h"
#include "oxygen/simulation/CodeExec.h"
#include "oxygen/simulation/LogDisplay.h"
#include "oxygen/simulation/Simulation.h"

bool ArchipelagoClient::startConnection()
{
	if (mSetupDone)
	{
		// Already connected?
		if (mSocket.isValid())
			return true;
		mSetupDone = false;
	}

	Sockets::startupSockets();
	
	if (!mSocket.connectTo("127.0.0.1", 38281))
	{
		LogDisplay::instance().setLogDisplay("Couldn't connect to Archipelago", 10.0f);
		return false;
	}
	
	LogDisplay::instance().setLogDisplay("Now connected to Archipelago!", 10.0f);
	std::string request = 
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1:38281\r\n"
		"Connection: Upgrade\r\n"
		"Upgrade: websocket\r\n"
		"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"\r\n";
	sendStr(request);
	
	// Done
	mSetupDone = true;
	return true;
}

void ArchipelagoClient::stopConnection()
{
	mSocket.close();
	mSetupDone = false;
}

bool ArchipelagoClient::isConnected()
{
	/*
	TCPSocket::ReceiveResult result;
	bool data = mSocket.receiveNonBlocking(result);
	if (!data || result.mBuffer.size() <= 0)
	{
		LogDisplay::instance().setLogDisplay("Connection was dropped", 10.0f);
		mSetupDone = false;
	}
	*/
		
	return mSetupDone;
}

void ArchipelagoClient::updateConnection(float timeElapsed)
{
	if (!mSetupDone)
		return;

	TCPSocket::ReceiveResult result;
	if (mSocket.receiveNonBlocking(result) && !result.mBuffer.empty())
	{
		std::string errors;
		Json::Value jsonRoot = rmx::JsonHelper::loadFromMemory(result.mBuffer, &errors);
		if (jsonRoot.isObject())
		{
			evaluateRequestJson(jsonRoot);
		}
	}
}

void ArchipelagoClient::sendResponse(lemon::StringRef message)
{
	if (message.isEmpty())
		return;
	
	std::string response = std::string(message.getString());
	ArchipelagoClient::sendStr(response);
}

void ArchipelagoClient::sendStr(std::string msg)
{
	mSocket.sendData((const uint8*)msg.c_str(), msg.length() + 1);
}

void ArchipelagoClient::evaluateRequestJson(const Json::Value& requestJson)
{
	LogDisplay::instance().setLogDisplay("Received a message from the Archipelago server.", 8.0f);
	JsonHelper jsonHelper(requestJson);
}

