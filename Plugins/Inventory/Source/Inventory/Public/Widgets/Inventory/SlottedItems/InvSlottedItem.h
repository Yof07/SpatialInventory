// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/SlottedItems/InvSlottedItem.h
#pragma once

#include "CoreMinimal.h"
#include "IAutomationReport.h"
#include "Blueprint/UserWidget.h"
#include "Items/InvInventoryItem.h"
#include "InvSlottedItem.generated.h"

class UTextBlock;
class UInvInventoryItem;
class UImage;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSlottedItemClicked, int32, GridIndex, const FPointerEvent&, MouseEvent);

/**
 * 单网格展示控件(放入网格)
 */
UCLASS()
class INVENTORY_API UInvSlottedItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	void SetIsStackable(bool bStackable) { bIsStackable = bStackable; }
	bool IsStackable() const {return bIsStackable;}
	
	UImage* GetImageIcon() const { return Image_Icon; }
		
	void SetGridIndex(const int32 Index) { GirdIndex = Index; }
	int32 GetGridIndex() const { return GirdIndex; }
	
	void SetGridDimensions(const FIntPoint& Dimensions) { GridDimensions = Dimensions; }
	FIntPoint GetGridDimensions() const { return GridDimensions; }

	void SetInventoryItem(UInvInventoryItem* Item) { InventoryItem = Item; }
	UInvInventoryItem* GetInventoryItem() const { return InventoryItem.Get(); }

	void SetImageBrush(const FSlateBrush& Brush) const;

	void UpdateStackCount(int32 StackCount);

public:
	FSlottedItemClicked OnSlottedItemClicked;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;

	int32 GirdIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInvInventoryItem> InventoryItem;
	bool bIsStackable = false; // 是否堆叠，即是否可以改变右下角物品数量数字
};
