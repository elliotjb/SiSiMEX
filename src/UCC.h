#pragma once
#include "Agent.h"

class UCC :
	public Agent
{
public:

	// Constructor and destructor
	UCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId);
	~UCC();

	// Agent methods
	void update() override;
	void stop() override;
	UCC* asUCC() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	bool SendPacketToUCP(int dstId);

	// TODO
	// Getters
	uint16_t contributedItemId() const { return _contributedItemId; }
	uint16_t constraintItemId() const { return _constraintItemId; }
	bool NegotiationAccepted() const { return _negotiationAccepted; }

private:
	uint16_t _contributedItemId;
	uint16_t _constraintItemId;

	bool _negotiationAccepted;
};

