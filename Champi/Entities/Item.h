#pragma once

#include <string>

using namespace std;

class Item
{
public:
	Item();
	virtual ~Item();

	int id; // #
	string name; // Name
	int icon; // Icon
};