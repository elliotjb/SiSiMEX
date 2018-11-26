#pragma once
#include "Agent.h"

// Forward declaration
class UCP;
using UCPPtr = std::shared_ptr<UCP>;

class MCP :
	public Agent
{
public:

	// Constructor and destructor
	MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth);
	~MCP();

	// Agent methods
	void update() override;
	void stop() override;
	MCP* asMCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	// Getters
	uint16_t requestedItemId() const { return _requestedItemId; }
	uint16_t contributedItemId() const { return _contributedItemId; }

	// Whether or not the negotiation finished
	bool negotiationFinished() const;

	// Whether or not there was a negotiation agreement
	bool negotiationAgreement() const;

	bool Requestnegotiation();

	// It returns the search depth of this MCP
	unsigned int searchDepth() const { return _searchDepth; }
	bool NegotiationAccepted() const { return _negotiationAccepted; }

private:

	bool queryMCCsForItem(int itemId);

	void createChildUCP(const AgentLocation& location);

	void destroyChildUCP();

	uint16_t _requestedItemId;
	uint16_t _contributedItemId;

	int _mccRegisterIndex; /**< Iterator through _mccRegisters. */
	std::vector<AgentLocation> _mccRegisters; /**< MCCs returned by the YP. */

	unsigned int _searchDepth;

	bool _negotiationAccepted;

	// TODO: Add extra attributes and methods?
	UCPPtr _ucp; /**< Child UCP. */
};

