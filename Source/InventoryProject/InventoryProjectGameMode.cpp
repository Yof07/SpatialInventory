// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryProjectGameMode.h"
#include "InventoryProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AInventoryProjectGameMode::AInventoryProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AInventoryProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		LogAllPlayerControllerPawnPairs();
	}
}

void AInventoryProjectGameMode::LogAllPlayerControllerPawnPairs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = Cast<APlayerController>(It->Get());
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		UE_LOG(LogTemp, Log, TEXT("PlayerController: %s  -> Pawn: %s"),
			   *PC->GetName(),
			   Pawn ? *Pawn->GetName() : TEXT("None"));
	}
}
