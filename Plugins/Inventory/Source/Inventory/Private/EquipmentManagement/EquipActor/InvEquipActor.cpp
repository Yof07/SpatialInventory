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

	// DOREPLIFETIME(AInvEquipActor, PredictedClientSequence, COND_OwnerOnly);
}

// void AInvEquipActor::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	if (PredictedClientSequence != 0)
// 	{
// 		APlayerController* PC = Cast<APlayerController>(GetOwner());
// 		if (IsValid(PC) && PC->IsLocalController()) // 当前为玩家操纵的客户端
// 		{
// 			if (UInvInventoryComponent* InvComp = PC->FindComponentByClass<UInvInventoryComponent>())
// 			{
// 				InvComp->DestroyPredictedActorsBySequence(PredictedClientSequence);
// 			}
// 		}
// 	}
// }
//
// void AInvEquipActor::OnRep_PredictedClientSequence() const
// {
// 	APlayerController* PC = Cast<APlayerController>(GetOwner());
// 	if (IsValid(PC) && PC->IsLocalController()) // 当前为玩家操纵的客户端
// 	{
// 		if (UInvInventoryComponent* InvComp = PC->FindComponentByClass<UInvInventoryComponent>())
// 		{
// 			InvComp->DestroyPredictedActorsBySequence(PredictedClientSequence);
// 		}
// 	}
// }



