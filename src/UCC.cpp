#include "UCC.h"

// TODO: Make an enum with the states
enum State
{
	ST_INIT,
	//
	ST_IDLE,
	//
	ST_REQUESTING_ITEM,
	ST_RESOLVING_CONSTRAINT,
	ST_SENDING_CONSTRAINT,
	//
	ST_FINISHED
};

UCC::UCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	// TODO: Save input parameters
	setState(ST_INIT);
}

UCC::~UCC()
{
}

void UCC::update()
{
	switch (state())
	{
	case ST_INIT:
		setState(ST_IDLE);
		break;
	case ST_REQUESTING_ITEM:
		break;
	case ST_RESOLVING_CONSTRAINT:
		break;
	case ST_SENDING_CONSTRAINT:
		break;
	case ST_FINISHED:
		break;
	default:
		break;
	}
}

void UCC::stop()
{
	destroy();
}

void UCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::RequestItem:
		if (state() == ST_IDLE)
		{

			// Create message header and data
			PacketHeader packetHead;
			packetHead.packetType = PacketType::RequestConstraint;
			packetHead.srcAgentId = id();
			packetHead.dstAgentId = packetHead.srcAgentId;

			//PacketItemRequest packetData;
			PacketConstraintRequest packetData;
			packetData.itemId = constraintItemId();

			// Serialize message
			OutputMemoryStream stream;
			packetHead.Write(stream);
			packetData.Write(stream);

			socket->SendPacket(stream.GetBufferPtr(), stream.GetSize());
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RequestItem was unexpected.";

		}
	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}


