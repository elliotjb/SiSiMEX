#pragma once
#include "Item.h"
#include <memory>

class Node
{
public:

	// Constructor and destructor
	Node(int id);
	~Node();

	// Initialization of its item list
	void initialize();

	// Getters
	int id() { return _id; }
	ItemList &itemList() { return _itemList; }
	const ItemList &itemList() const { return _itemList; }

private:

	int _id; /**< Id of this node. */
	ItemList _itemList; /**< All items owned by this node. */
};

using NodePtr = std::shared_ptr<Node>;
