// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/Spatial/InvSpatialInventory.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/CanvasPanel.h"
#include "Widgets/Inventory/InventoryBase/InvInventoryBase.h"
#include "InvSpatialInventory.generated.h"

class UInvEquippedSlottedItem;
class UInvEquippedGridSlot;
class UInvItemDescription;
class UCanvasPanel;
class UInvItemComponent;
class UButton;
class UWidgetSwitcher;
class UInvInventoryGrid;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvSpatialInventory : public UInvInventoryBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override; // UUserWidget虚函数

	virtual FInvSlotAvailabilityResult HasRoomForItem(UInvItemComponent* ItemComponent) const override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 生成展示物品描述 */
	virtual void OnItemHovered(UInvInventoryItem* Item) override;
	virtual void OnItemUnhovered() override;
	virtual bool HasHoverItem() const override;
	virtual UInvHoverItem* GetHoverItem() const override;
	
private:
	/** 按键回调 */
	UFUNCTION()
	void ShowEquippables(); 

	/** 按键回调 */
	UFUNCTION()
	void ShowConsumables(); 

	/** 按键回调 */
	UFUNCTION()
	void ShowCraftables(); 

	/** 装备网格点击回调 */
	UFUNCTION()
	void EquippedGridSlotClicked(UInvEquippedGridSlot* EquippedGridSlot, const FGameplayTag& EquipmentTypeTag);

	/** 装备物品点击回调 */
	UFUNCTION()
	void EquippedSlottedItemClicked(UInvEquippedSlottedItem* EquippedSlottedItem);

	// 说明切换并显示哪个网格，禁用哪些按钮，允许使用哪些按钮
	void SetActiveGrid(UInvInventoryGrid* Grid, UButton* Button);

	void DisableButton(UButton* Button);

	/** 懒加载物品描述 */
	UInvItemDescription* GetItemDescription();

	void SetItemDescriptionSizeAndPosition(UInvItemDescription* Description, UCanvasPanel* Canvas) const;

	bool CanEquipHoverItem(UInvEquippedGridSlot* EquippedGridSlot, const FGameplayTag& EquipmentTypeTag) const;

	virtual float GetTileSize() const override;

	UInvEquippedGridSlot* FindSlotWithEquippedItem(UInvInventoryItem* EquippedItem) const;

	void ClearSlotOfItem(UInvEquippedGridSlot* EquippedGridSlot);

	void RemoveEquippedSlottedItem(UInvEquippedSlottedItem* EquippedSlottedItem);

	void MakeEquippedSlottedItem(UInvEquippedSlottedItem* EquippedSlottedItem, UInvEquippedGridSlot* EquippedGridSlot, UInvInventoryItem* ItemToEquip);

	void BroadcastSlotClickedDelegates(UInvInventoryItem* ItemToEquip, UInvInventoryItem* ItemToUnequip) const;
	
private:
	/**
	 * 一种容器，可以在多个子页面之间切换，只显示当前激活那个
	 */
	UPROPERTY(meta = (BindWidget)) // 该c++的派生蓝图类必须有相同类型且相同名字(Switcher)的成员控件，设计的意义？
	TObjectPtr<UWidgetSwitcher> Switcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInvInventoryGrid> Grid_Equippables; // 可装备品网格

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInvInventoryGrid> Grid_Consumables; // 可消耗品网格

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInvInventoryGrid> Grid_Craftables; // 可合成品网格

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Equippables;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consumables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Craftables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInvItemDescription> ItemDescriptionClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DescriptionTimerDelay = 0.5f;

	UPROPERTY()
	TObjectPtr<UInvItemDescription> ItemDescription;
	
	UPROPERTY()
	TArray<TObjectPtr<UInvEquippedGridSlot>> EquippedGridSlots;

	TWeakObjectPtr<UInvInventoryGrid> ActiveGrid;

	FTimerHandle DescriptionTimer;
}; 

