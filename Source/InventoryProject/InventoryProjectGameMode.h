// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InventoryProjectGameMode.generated.h"

UCLASS(minimalapi)
class AInventoryProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInventoryProjectGameMode();

	virtual void BeginPlay() override;

	/** 打印所有PC及其对应的Pawn */
	UFUNCTION(Exec)
	void LogAllPlayerControllerPawnPairs();
};



