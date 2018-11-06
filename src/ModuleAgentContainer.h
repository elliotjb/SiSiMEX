#pragma once

#include "Module.h"
#include <memory>
#include <vector>

class Node;
class Agent;
using AgentPtr = std::shared_ptr<Agent>;
class MCC;
using MCCPtr = std::shared_ptr<MCC>;

class ModuleAgentContainer : public Module
{
public:

	// Constructor and destructor
	ModuleAgentContainer();
	~ModuleAgentContainer();

	// Agent creation methods
	MCCPtr createMCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId);

	// Getters
	AgentPtr getAgent(int agentId);
	bool empty() const;

	// Update
	bool update() override;

	// Post update
	bool postUpdate() override;

	// Finalize
	bool stop() override;

private:

	// Setters
	void addAgent(AgentPtr agent);

	std::vector<AgentPtr> _agentsToAdd; /**< Agents to add. */
	std::vector<AgentPtr> _agents; /**< Array of agents. */
};