#include "MCC.h"
#include "UCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	
	// TODO: Other states
	ST_NEGOTIATING,

	ST_FINISHED = 10
};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	setState(ST_INIT);
}


MCC::~MCC()
{
}

void MCC::update()
{
	switch (state())
	{
	case ST_INIT:
		if (registerIntoYellowPages()) 
		{
			setState(ST_REGISTERING);
		}
		else 
		{
			setState(ST_FINISHED);
		}
		break;

	case ST_REGISTERING:
		// See OnPacketReceived()
		break;

	case ST_NEGOTIATING:
		if (_ucc.get())
		{
			if (_ucc.get()->state() == 10)
			{
				if (_ucc.get()->NegotiationAccepted())
				{
					// Negotiation Accepted -------
					setState(ST_FINISHED);
				}
				else
				{
					// Negotiation Canceled -------
					setState(ST_IDLE);
					if (_ucc != nullptr)
						_ucc->stop();
				}
				node()->itemList().UpdateItemUsed(_contributedItemId, false);
			}
		}
		break;

		// TODO: Handle other states

	case ST_FINISHED:
	{

	}
	}
}

void MCC::stop()
{
	// Destroy hierarchy below this agent (only a UCC, actually)
	destroyChildUCC();
	destroy();
	unregisterFromYellowPages();
	setState(ST_FINISHED);
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::RegisterMCCAck:
		if (state() == ST_REGISTERING)
		{
			setState(ST_IDLE);
			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RegisterMCCAck was unexpected.";
		}
		break;

	// TODO: Handle other packets
	case PacketType::RequestNegotiation:
	{
		PacketHeader outPacketHead;
		PacketRequestNegotiation packetData;
		OutputMemoryStream outStream;

		outPacketHead.packetType = PacketType::RequestNegotiationResponse;
		outPacketHead.srcAgentId = id();
		outPacketHead.dstAgentId = packetHeader.srcAgentId;

		if(state() == ST_IDLE)
		{
			if (node()->itemList().isItemsWithIdUsed(_contributedItemId))
			{
				packetData.accepted = false;

				outPacketHead.Write(outStream);
				packetData.Write(outStream);
				socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());

				break;
			}
			else
			{
				node()->itemList().UpdateItemUsed(_contributedItemId, true);
			}

			packetData.accepted = true;

			createChildUCC();

			//Set AgentLocation
			packetData.uccLocation.hostIP = socket->RemoteAddress().GetIPString();
			packetData.uccLocation.hostPort = LISTEN_PORT_AGENTS;
			packetData.uccLocation.agentId = _ucc->id();

			setState(ST_NEGOTIATING);
		}
		else
		{
			packetData.accepted = false;
		}

		outPacketHead.Write(outStream);
		packetData.Write(outStream);
		socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());

		break;
	}

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCC::isIdling() const
{
	return state() == ST_IDLE;
}

bool MCC::negotiationFinished() const
{
	return state() == ST_FINISHED;
}

bool MCC::negotiationAgreement() const
{
	// If this agent finished, means that it was an agreement
	// Otherwise, it would return to state ST_IDLE
	return negotiationFinished();
}

bool MCC::registerIntoYellowPages()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RegisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketRegisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	return sendPacketToYellowPages(stream);
}

void MCC::unregisterFromYellowPages()
{
	// Create message
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UnregisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketUnregisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	sendPacketToYellowPages(stream);
}

void MCC::createChildUCC()
{
	Node* newNode = new Node(App->agentContainer->allAgents().size());
	_ucc = App->agentContainer->createUCC(node(), contributedItemId(), constraintItemId());
}

void MCC::destroyChildUCC()
{
	// TODO: Destroy the unicast contributor child
	if(_ucc != nullptr)
		_ucc->stop();
}
