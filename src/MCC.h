#pragma once
#include "Agent.h"

class MCC :
	public Agent
{
public:

	// Constructor and destructor
	MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId = NULL_ITEM_ID);
	~MCC();

	// Agent methods
	void start() override;
	void update() override;
	void stop() override;
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	// Getters
	uint16_t contributedItemId() const { return _contributedItemId; }
	uint16_t constraintItemId() const { return _constraintItemId; }

	// Whether or not the negotiation finished
	bool negotiationFinished() const;

	// Whether or not there was a negotiation agreement
	bool negotiationAgreement() const;

private:

	bool registerIntoYellowPages();
	
	bool unregisterFromYellowPages();

	uint16_t _contributedItemId; /**< The contributed item. */
	uint16_t _constraintItemId; /**< The constraint item. */
};
