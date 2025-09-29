// Fill out your copyright notice in the Description page of Project Settings.
// EquipmentComponent/EquipActor/InvEquipActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "InvEquipActor.generated.h"

class UInvInventoryItem;

UCLASS()
class INVENTORY_API AInvEquipActor : public AActor
{
	GENERATED_BODY()

public:
	AInvEquipActor();
	
	FGameplayTag GetEquipmentType() const { return EquipmentType; }
	void SetEquipmentType(FGameplayTag Type) { EquipmentType = Type; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag EquipmentType
};
