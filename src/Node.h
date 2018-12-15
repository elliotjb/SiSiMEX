#pragma once
#include "ItemList.h"
#include <memory>
#include <string>

enum TypeUser
{
	DEFAULT,
	FORCE,
	AGILITY,
	INTELLIGENCE
};

class Node
{
public:

	// Constructor and destructor
	Node(int id);
	Node(int id, std::string nameUser, TypeUser type);
	~Node();

	// Getters
	int id() { return _id; }
	ItemList &itemList() { return _itemList; }
	const ItemList &itemList() const { return _itemList; }
	std::string GetName() const
	{
		return nameUser;
	}
	TypeUser GetType() const
	{
		return type;
	}

private:

	int _id; /**< Id of this node. */
	std::string nameUser;
	TypeUser type;
	ItemList _itemList; /**< All items owned by this node. */
};

using NodePtr = std::shared_ptr<Node>;
