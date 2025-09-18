// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/InventoryBase/InvInventoryBase.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/InvGridTypes.h"
#include "InvInventoryBase.generated.h"

class UInvHoverItem;
class UInvItemComponent;
/**
 * 仓库基类
 */
UCLASS()
class INVENTORY_API UInvInventoryBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// 检查背包是否能放下该物品
	virtual FInvSlotAvailabilityResult HasRoomForItem(UInvItemComponent* ItemComponent) const { return FInvSlotAvailabilityResult(); }

	/** 生成物品描述 */
	virtual void OnItemHovered(UInvInventoryItem* Item) {}

	/** 移除物品描述 */
	virtual void OnItemUnhovered() {}

	virtual bool HasHoverItem() const { return false; }

	virtual UInvHoverItem* GetHoverItem() const { return nullptr; }

	virtual float GetTileSize() const { return 0.0f; }
};
