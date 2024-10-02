// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

ADVANCEDMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogAdvancedMovement, Log, All);

class FAdvancedMovementModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
