// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/HoverItem/InvHoverItem.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "InvHoverItem.generated.h"

/**
 * 当一个仓库中的物品被点击时，会悬浮并跟随鼠标
 */

class UTextBlock;
class UInvInventoryItem;
class UImage;
	
UCLASS()
class INVENTORY_API UInvHoverItem : public UUserWidget
{	
	GENERATED_BODY()
	
public:
	void SetImageBrush(const FSlateBrush& Brush) const;
	
	void UpdateStackCount(const int32 Count);
	
	FGameplayTag GetItemType() const;
	
	int32 GetStackCount() const { return StackCount; }
	
	bool IsStackable() const { return bIsStackable; }
	void SetIsStackable(const bool bStacks);
    
	int32 GetPreviousGridIndex() const { return PreviousGridIndex; }
	void SetPreviousGridIndex(const int32 Index) { PreviousGridIndex = Index; }
    
	FIntPoint GetGridDimensions() const { return GridDimensions; }
	void SetGridDimensions(const FIntPoint& Dimensions) { GridDimensions = Dimensions; }
	
	UInvInventoryItem* GetInventoryItem() const;
	void SetInventoryItem(UInvInventoryItem* Item);
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;
    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;
    
	int32 PreviousGridIndex = 0;
    
	FIntPoint GridDimensions;
    
	TWeakObjectPtr<UInvInventoryItem> InventoryItem;
    
	bool bIsStackable = false;
    
	int32 StackCount = 0;
};
