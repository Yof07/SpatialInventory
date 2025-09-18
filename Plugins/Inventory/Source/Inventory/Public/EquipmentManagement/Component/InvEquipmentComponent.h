// Fill out your copyright notice in the Description page of Project Settings.
// EquipmentComponent/Component/InvEquipmentComponent.h
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Components/ActorComponent.h"
#include "InvEquipmentComponent.generated.h"


struct FInvItemManifest;
struct FInvEquipmentFragment;
class AInvEquipActor;
class UInvInventoryItem;
class UInvInventoryComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInvEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void SetOwningSkeletalMesh(USkeletalMeshComponent* OwningMesh);

	void SetIsProxy(bool bProxy) { bIsProxy = bProxy; }

	void InitializeOwner(APlayerController* PlayerController);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/** 回调，获取EquipmentFragment碎片并调用其功能 */
	UFUNCTION()
	void OnItemEquipped(UInvInventoryItem* EquippedItem);

	/** 回调 */
	UFUNCTION()
	void OnItemUnequipped(UInvInventoryItem* UnequippedItem);

	UFUNCTION()
	void OnPossessedPawnChange(APawn* OldPawn, APawn* NewPawn);

	/**
	 * 初始化InventoryComponent字段并对其OnItemEquipped和OnItemUnequipped进行订阅；
	 * 无论是角色身上的EquipmentComponent还是ProxyMesh上的EquipmentComponent都可以对上面的委托进行订阅，
	 *		这样当这两个委托在广播的时候，不同的EquipmentComponent都可以对相应的OwningSkeletalMesh成员进行装备穿戴
	 * */
	void InitInventoryComponent();

	void InitPlayerController();

	AInvEquipActor* SpawnEquippedActor(FInvEquipmentFragment* EquipmentFragment, const FInvItemManifest& Manifest, USkeletalMeshComponent* AttachMesh);

	AInvEquipActor* FindEquippedActor(const FGameplayTag& EquipmentTypeTag);

	void RemoveEquippedActor(const FGameplayTag& EquipmentTypeTag);

private:
	UPROPERTY()
	TArray<TObjectPtr<AInvEquipActor>> EquippedActors;
	
	TWeakObjectPtr<UInvInventoryComponent> InventoryComponent;
	TWeakObjectPtr<APlayerController> OwningPlayerController;

	/** 把装备挂载到该骨骼组件所拥有的骨骼上 */ 
	TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMesh;

	bool bIsProxy = false;
};
