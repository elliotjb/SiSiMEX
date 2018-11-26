#pragma once
#include "Agent.h"

// Forward declaration
class MCP;
using MCPPtr = std::shared_ptr<MCP>;

class UCP :
	public Agent
{
public:

	// Constructor and destructor
	UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLoc, unsigned int searchDepth);
	~UCP();

	// Agent methods
	void update() override;
	void stop() override;
	UCP* asUCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	bool SendPacketToUCC();

	// Getters
	uint16_t requestedItemId() const { return _requestedItemId; }
	uint16_t contributedItemId() const { return _contributedItemId; }
	bool NegotiationAccepted() const { return _negotiationAccepted; }

private:
	uint16_t _requestedItemId;
	uint16_t _contributedItemId;
	unsigned int _searchDepth;

	bool _negotiationAccepted;

	AgentLocation _uccLocation;

	MCPPtr _mcp; /**< Child MPC. */
};

