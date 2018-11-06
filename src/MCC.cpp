#include "MCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"

// With these states you have enough so far...
enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	ST_UNREGISTERING,
	ST_FINISHED
};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
}


MCC::~MCC()
{
}

void MCC::start()
{
	// TODO: Set the initial state
	setState(ST_INIT);
}

void MCC::update()
{
	switch (state())
	{
		// TODO:
		// - Register or unregister into/from YellowPages depending on the state
		//       Use the functions registerIntoYellowPages and unregisterFromYellowPages
		//       so that this switch statement remains clean and readable
		// - Set the next state when needed ...
	case ST_INIT:
	{
		if (registerIntoYellowPages()) {
			setState(ST_REGISTERING);
		}
		else {
			setState(ST_FINISHED);
		}
		break;
	}
	case ST_REGISTERING:
	{
		break;
	}
	case ST_IDLE:
	{
		break;
	}
	case ST_UNREGISTERING:
	{

		break;
	}
	case ST_FINISHED:
	{
		//destroy();
		break;
	}
	}
}

void MCC::stop()
{
	unregisterFromYellowPages();
	setState(ST_UNREGISTERING);
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	// Taking the state into account, receive and deserialize packets (Ack packets) and set next state
	if (state() == ST_REGISTERING && packetHeader.packetType == PacketType::RegisterMCCAck)
	{
		// TODO: Set the next state (Idle in this case)
		setState(ST_IDLE);

		// TODO: Disconnect the socket (we don't need it anymore)
		socket->Disconnect();
	}

	// TODO: Do the same for unregistering

	// Taking the state into account, receive and deserialize packets (Ack packets) and set next state
	if (state() == ST_UNREGISTERING && packetHeader.packetType == PacketType::UnregisterMCCAck)
	{
		// TODO: Set the next state (Idle in this case)
		setState(ST_IDLE);

		// TODO: Disconnect the socket (we don't need it anymore)
		socket->Disconnect();
	}
}

bool MCC::negotiationFinished() const
{
	return false;
}

bool MCC::negotiationAgreement() const
{
	return false;
}

bool MCC::registerIntoYellowPages()
{
	// TODO: Create a PacketHeader (make it in Packets.h)
	PacketHeader pck = PacketHeader();
	pck.packetType = PacketType::RegisterMCC;
	pck.srcAgentId = Agent::id();

	// TODO: Create a PacketRegisterMCC (make it in Packets.h)
	PacketRegisterMCC pckMCC = PacketRegisterMCC();
	pckMCC.itemId = contributedItemId();

	// TODO: Serialize both packets into an OutputMemoryStream
	OutputMemoryStream stream;
	pck.Write(stream);
	pckMCC.Write(stream);

	// TODO: Send the stream (Agent::sendPacketToYellowPages)
	Agent::sendPacketToYellowPages(stream);

	return true;
}

bool MCC::unregisterFromYellowPages()
{
	// TODO: Create a PacketHeader (make it in Packets.h)
	PacketHeader pck = PacketHeader();
	pck.packetType = PacketType::UnregisterMCC;
	pck.srcAgentId = Agent::id();

	// TODO: Create a PacketUnregisterMCC (make it in Packets.h)
	PacketUnregisterMCC pckMCC = PacketUnregisterMCC();
	pckMCC.itemId = contributedItemId();

	// TODO: Serialize both packets into an OutputMemoryStream
	OutputMemoryStream stream;
	pck.Write(stream);
	pckMCC.Write(stream);

	// TODO: Send the stream (Agent::sendPacketToYellowPages)
	Agent::sendPacketToYellowPages(stream);

	return true;
}
