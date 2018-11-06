#pragma once

#include "ModuleNetworkManager.h"


bool ModuleNetworkManager::init()
{
	SocketUtil::StaticInit();

	return true;
}

bool ModuleNetworkManager::preUpdate()
{
	return true;
}

bool ModuleNetworkManager::postUpdate()
{
	const int timeoutMillis = 0;
	HandleSocketOperations(timeoutMillis);

	return true;
}

bool ModuleNetworkManager::stop()
{
	Finalize();

	return true;
}

bool ModuleNetworkManager::cleanUp()
{
	SocketUtil::CleanUp();

	return true;
}
