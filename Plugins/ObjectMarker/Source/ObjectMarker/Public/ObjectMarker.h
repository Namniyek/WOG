// Copyright BabbyGames All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FObjectMarkerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
