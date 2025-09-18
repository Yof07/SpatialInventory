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

	// virtual void BeginPlay() override;
	//
	// UFUNCTION()
	// void OnRep_PredictedClientSequence() const;

private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag EquipmentType;
	
	// UPROPERTY(ReplicatedUsing = OnRep_PredictedClientSequence, VisibleAnywhere, Category="Inventory")
	// uint32 PredictedClientSequence = 0; // 如果为0，则表示是在Server端生成的，非客户端预测时生成的
};

// UENUM(BlueprintType)
// enum class EEquipAction : uint8
// {
// 	Equip,
// 	Unequip
// };
//
// USTRUCT()
// struct FPendingEquipEntry
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY()
// 	uint32 ClientSequence = 0;
//
// 	UPROPERTY()
// 	TObjectPtr<UInvInventoryItem> Item = nullptr;
//
// 	/** 装备槽位标识，如头盔槽、装甲槽、腿甲槽、手部槽等 */
// 	UPROPERTY()
// 	FGameplayTag SlotTag;
//
// 	UPROPERTY()
// 	EEquipAction Action = EEquipAction::Equip;
//
// 	UPROPERTY(Transient)
// 	TWeakObjectPtr<AInvEquipActor> PredictedProxyActor;
//
// 	UPROPERTY(Transient)
// 	TWeakObjectPtr<AInvEquipActor> PredictedCharacterActor;
//
// 	/** 时间戳 */
// 	UPROPERTY()
// 	float TimeStamp = 0.0;
// };
