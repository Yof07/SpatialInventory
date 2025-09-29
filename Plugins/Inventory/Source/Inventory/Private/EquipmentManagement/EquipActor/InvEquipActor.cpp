// Fill out your copyright notice in the Description page of Project Settings.
// EquipmentComponent/EquipActor/InvEquipActor.cpp

#include "EquipmentManagement/EquipActor/InvEquipActor.h"

#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AInvEquipActor::AInvEquipActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AInvEquipActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}



