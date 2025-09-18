// Fill out your copyright notice in the Description page of Project Settings.
// Items/Components/InvItemComponent.cpp

#include "Items/Components/InvItemComponent.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UInvItemComponent::UInvItemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	PickUpMessage = TEXT("E - 拾取"); 

	SetIsReplicatedByDefault(true);
}

void UInvItemComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemManifest);
}

void UInvItemComponent::PickedUp()
{
	if (IsValid(GetOwner()))
	{
		OnPickedUp();
		GetOwner()->Destroy();
	}
	
}

void UInvItemComponent::InitItemManifest(FInvItemManifest CopyOfManifest)
{
	ItemManifest = CopyOfManifest;
}





