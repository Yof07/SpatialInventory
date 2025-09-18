// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/GridSlots/InvEquippedGridSlot.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvGridSlot.h"
#include "InvEquippedGridSlot.generated.h"


class UOverlay;
class UInvEquippedSlottedItem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquippedGridSlotClicked, UInvEquippedGridSlot*, GridSlot,
                                             const FGameplayTag&, EquipmentTypeTag);

/**
 * 
 */
UCLASS()
class INVENTORY_API UInvEquippedGridSlot : public UInvGridSlot
{
	GENERATED_BODY()

public:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 将EquippedSlottedItem绘制在EquippedGridSlot上 */
	UInvEquippedSlottedItem* OnItemEquipped(UInvInventoryItem* Item, const FGameplayTag& EquipmentTag, float TileSize);

	void SetEquippedSlottedItem(UInvEquippedSlottedItem* Item) { EquippedSlottedItem = Item; }
	
public:
	FEquippedGridSlotClicked EquippedGridSlotClicked;

private:
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (Categories = "GameItems.Equipment"))
	FGameplayTag EquipmentTypeTag;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_GrayedOutIcon;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInvEquippedSlottedItem> EquippedSlottedItemClass;

	UPROPERTY()
	TObjectPtr<UInvEquippedSlottedItem> EquippedSlottedItem;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> Overlay_Root;
};
