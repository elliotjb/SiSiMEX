#pragma once

#include "Module.h"
#include "net/Net.h"
#include "Node.h"
#include "MCC.h"

class ModuleNodeCluster : public Module, public TCPNetworkManagerDelegate
{
public:

	// Virtual methods from parent class Module

	bool init() override;

	bool start() override;

	bool update() override;

	bool updateGUI() override;

	bool cleanUp() override;

	bool stop() override;


	// TCPNetworkManagerDelegate virtual methods

	void OnAccepted(TCPSocketPtr socket) override;

	void OnPacketReceived(TCPSocketPtr socket, InputMemoryStream &stream) override;

	void OnDisconnected(TCPSocketPtr socket) override;

private:

	bool startSystem();

	void runSystem();

	void stopSystem();


	void spawnMCC(int nodeId, int contributedItemId, int constraintItemId = NULL_ITEM_ID);



	std::vector<NodePtr> _nodes; /**< Array of nodes spawn in this host. */

	std::vector<MCC*> _mccs; /**< Multicast contributors. */

	int state = 0; /**< State machine. */
};
