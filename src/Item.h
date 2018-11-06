#pragma once

#include "net/Net.h"


/**
 * This constant defines which is the maximum number of
 * items of the catalogue. Items will be identified by an
 * index between 0 and MAX_ITEMS - 1.
 */
static const int MAX_ITEMS = 5;


/**
 * A single item and its properties.
 */
class Item
{
public:

	// Constructors and destructor
	Item();
	Item(int id);
	~Item();

	// Item identifier
	int id() const { return _id; }

	// Comparison operator
	bool operator<(const Item &item) { return _id < item._id; }

private:
	int _id; /**< Item identifier. */
};

/**
 * A list of items.
 */
class ItemList
{
public:

	// Constructor and destructor
	ItemList();
	~ItemList();

	// It initializes the item list randomly
	void randomInitialization();

	// Methods to add and remove items to/from the list
	void addItem(const Item &item);
	void removeItem(int itemId);

	// It returns the inner std::list of items
	const std::list<Item> &items() const { return _items; }

	// It returns a new list with all the repeated items
	ItemList getSpareItems() const;

	// It returns a new list with the items that are missing
	ItemList getMissingItems() const;


private:

	std::list<Item> _items; /**< A list of items. */
};
