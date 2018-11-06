#include "Item.h"
#include "Globals.h"
#include <algorithm>


Item::Item() : _id(NULL_ITEM_ID)
{
}

Item::Item(int id) : _id(id)
{
}

Item::~Item()
{
}


ItemList::ItemList()
{
}


ItemList::~ItemList()
{
}

void ItemList::randomInitialization()
{
	// Create random items
	for (int i = 0; i < MAX_ITEMS; ++i) {
		int itemId = rand() % MAX_ITEMS;
		Item item(itemId);
		addItem(item);
	}

	// Sort items
	_items.sort();
}

void ItemList::addItem(const Item &item)
{
	if (item.id() != NULL_ITEM_ID) {
		_items.push_back(item);
	}
}

void ItemList::removeItem(int itemId)
{
	for (auto it = _items.begin(); it != _items.end(); ++it)
	{
		const Item &item(*it);
		if (item.id() == itemId) {
			_items.erase(it);
			break;
		}
	}
}

ItemList ItemList::getSpareItems() const
{
	ItemList spareItems;
	std::set<int> existingItems;
	for (auto item : _items) {
		auto itemId = item.id();
		if (existingItems.find(itemId) != existingItems.end()) {
			spareItems.addItem(item);
		} else {
			existingItems.insert(itemId);
		}
	}
	return spareItems;
}

ItemList ItemList::getMissingItems() const
{
	std::set<int> existingItems;
	for (auto item : _items) {
		existingItems.insert(item.id());
	}
	ItemList missingItems;
	for (int i = 0; i < MAX_ITEMS; ++i) {
		if (existingItems.find(i) == existingItems.end()) {
			missingItems.addItem(Item(i));
		}
	}
	return missingItems;
}
