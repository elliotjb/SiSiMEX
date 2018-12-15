#include "Node.h"


Node::Node(int pid) :
	_id(pid)
{}

Node::Node(int pid, std::string nameUser, TypeUser type) :
	_id(pid),
	nameUser(nameUser),
	type(type)
{
}




Node::~Node()
{
}
