// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/SlottedItems/InvEquippedSlottedItem.h
#pragma once

#include "CoreMinimal.h"
#include "InvSlottedItem.h"
#include "InvEquippedSlottedItem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquippedSlottedItemClicked, UInvEquippedSlottedItem*, SlottedItem);

/**
 * 
 */
UCLASS()
class INVENTORY_API UInvEquippedSlottedItem : public UInvSlottedItem
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void SetEquipmentTypeTag(const FGameplayTag& Tag) { EquipmentTypeTag = Tag; }
	FGameplayTag GetEquipmentTypeTag() const { return EquipmentTypeTag; }

public:
	FEquippedSlottedItemClicked OnEquippedSlottedItemClicked;
	
private:
	UPROPERTY()
	FGameplayTag EquipmentTypeTag;
};
