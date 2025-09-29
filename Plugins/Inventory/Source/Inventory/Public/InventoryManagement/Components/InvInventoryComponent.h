// Fill out your copyright notice in the Description page of Project Settings.
// InventoryManagement/Components/InvInventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "EquipmentManagement/EquipActor/InvEquipActor.h"
#include "InventoryManagement/FastArray/InvFastArray.h"
#include "Types/InvGridTypes.h"
#include "InvInventoryComponent.generated.h"

class AInvEquipActor;
class UInvItemComponent;
class UInvInventoryBase;
	
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange, UInvInventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackChange, const FInvSlotAvailabilityResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEquipStatusChange, UInvInventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEquipStatusChange_OwnerView, UInvInventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMenuToggled, bool, bOpen);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInvInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UInvInventoryComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** InventoryComponent尝试拾取物体并放入仓库中，拾取物体后把物体放入仓库的入口 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Inventory")
	void TryAddItem(UInvItemComponent* ItemComponent);
	
	/**
	 * 当客户端调用这个函数时，函数调用会被 封包 并发送到服务器上执行；
	 * Reliable 表示引擎会保证这次网络调用最终送达（会重传直到确认）
	 */
	UFUNCTION(Server, Reliable) 
	void Server_AddNewItem(UInvItemComponent* ItemComponent, int32 StackCount);
	
	UFUNCTION(Server, Reliable) 
	void Server_AddStacksToItem(UInvItemComponent* ItemComponent, int32 StackCount, int32 Remainder);
	
	UFUNCTION(Server, Reliable)
	void Server_DropItem(UInvInventoryItem* Item, int32 StackCount);
	
	UFUNCTION(Server, Reliable)
	void Server_ConsumeItem(UInvInventoryItem* Item);
	
	UFUNCTION(Server, Reliable)
	void Server_EquipSlotClicked(UInvInventoryItem* ItemToEquip, UInvInventoryItem* ItemToUnequip);
	
	UFUNCTION(Client, Reliable)
	void Client_OnOwnerViewEquipped(UInvInventoryItem* ItemToEquip, UInvInventoryItem* ItemToUnequip);
	
	void AddRepSubObj(UObject* SubObj);
	
	void SpawnDroppedItem(UInvInventoryItem* Item, int32 StackCount);
	
	UInvInventoryBase* GetInventoryMenu() const { return InventoryMenu; };

	bool IsMenuOpen() const { return bInventoryMenuOpen; }
	
	/** 打开或关闭仓库菜单 */ 
	void ToggleInventoryMenu();

	// void DestroyPredictedActorsBySequence(uint32 ClientSequence);
	//
	// void RollbackLocalEquip(uint32 ClientSequence);
	
protected:
	virtual void BeginPlay() override;

private:
	void ConstructInventory();

	void OpenInventoryMenu();

	void CloseInventoryMenu();
public:
	FInventoryItemChange OnItemAdded;
	FInventoryItemChange OnItemRemoved;
	FNoRoomInInventory NoRoomInInventory;
	FStackChange OnStackChange;
	FItemEquipStatusChange OnItemEquipped;
	FItemEquipStatusChange OnItemUnequipped;
	FItemEquipStatusChange_OwnerView OnItemEquipped_OwnerView;
	FItemEquipStatusChange_OwnerView OnItemUnequipped_OwnerView;
	FInventoryMenuToggled OnInventoryMenuToggled;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInvInventoryBase> InventoryMenuClass;

	UPROPERTY() // 添加UPROPERTY防止被垃圾回收
	TObjectPtr<UInvInventoryBase> InventoryMenu;

	/** 服务器修改 UPROPERTY(Replicated)，引擎会把新值复制到客户端并可触发 OnRep 回调, 当服务器修改这个数组时，客户端会收到同步数据 */
	UPROPERTY(Replicated)
	FInvInventoryFastArray InventoryList;

	TWeakObjectPtr<APlayerController> OwningController;

	bool bInventoryMenuOpen;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnAngleMin = -85.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnAngleMax = 85.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnDistanceMin = 10.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnDistanceMax = 50.f;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float RelativeSpawnElevation = 70.f;
};
