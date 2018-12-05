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
	ST_FINISHED = 10
};

UCC::UCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	// TODO: Save input parameters
	setState(ST_INIT);
	_negotiationAccepted = false;
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
		//?
		destroy();
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
	const PacketType packetType = packetHeader.packetType;

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
			packetHead.dstAgentId = packetHeader.srcAgentId;

			//PacketItemRequest packetData;
			PacketConstraintRequest packetData;
			packetData.itemId = constraintItemId();

			// Serialize message
			OutputMemoryStream outstream;
			packetHead.Write(outstream);
			packetData.Write(outstream);

			socket->SendPacket(outstream.GetBufferPtr(), outstream.GetSize());
		}
		else
		{
			wLog << "UCC 1 - OnPacketReceived() - PacketType::RequestItem was unexpected.";

		}
		break;
	case PacketType::AcceptNegotiation:
	{
		if (state() == ST_IDLE)
		{
			PacketAcceptNegotiation packetData;
			packetData.Read(stream);

			_negotiationAccepted = packetData.acceptedNegotiation;
			setState(ST_FINISHED);
		}
		break;
	}
	default:
	{
		int temp = (int)packetType;
		wLog << "UCC 2 - OnPacketReceived() - Unexpected PacketType." << temp;
		wLog << "UCC 2 - OnPacketReceived() - Unexpected PacketType.";
	}
	}
}


