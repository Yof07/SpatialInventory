// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/GridSlots/InvGridSlot.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvGridSlot.generated.h"

class UInvItemPopUp;
class UInvInventoryItem;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent, int32, GridIndex, const FPointerEvent&, MouseEvent);

UENUM(BlueprintType)
enum class EInvGridSlotState : uint8
{
	Occupied,
	Unoccupied,
	Selected,
	GrayedOut
};


/**
 * 
 */
UCLASS()
class INVENTORY_API UInvGridSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 鼠标移动到widget区域任意可点击/可交互位置时调用
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 鼠标离开widget区域时调用
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	// 鼠标在widget区域点击任意按钮
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	EInvGridSlotState GetGridSlotState() const { return GridSlotState; }
	
	int32 GetIndex() const { return TileIndex; }
	void SetIndex(const int32 Index) { TileIndex = Index; }
	
	int32 GetStackCount() const { return DisplayStackCount; }
	void SetStackCount(const int Count) { DisplayStackCount = Count; }
	
	int32 GetUpperLeftIndex() const { return UpperLeftIndex; }
	void SetUpperLeftIndex(const int32 Index) { UpperLeftIndex = Index; }
	
	TWeakObjectPtr<UInvInventoryItem> GetInventoryItem() const { return InventoryItem; }
	void SetInventoryItem(UInvInventoryItem* Item);
	
	bool IsAvailable() const { return bAvailable; }
	void SetAvailable(const bool bIsAvailable) { bAvailable = bIsAvailable; }
	
	void SetOccupiedTexture();
	void SetUnOccupiedTexture();
	void SetSelectedTexture();
	void SetGrayedOutTexture();
	
	void SetItemPopUp(UInvItemPopUp* PopUp);
	UInvItemPopUp* GetItemPopUp() const;
	
private:
	// ItemPopUp的销毁回调
	UFUNCTION()
	void OnItemPopUpDestruct(UUserWidget* Menu);
	
public:
	FGridSlotEvent GridSlotClicked;
	FGridSlotEvent GridSLotHovered;
	FGridSlotEvent GridSLotUnHovered;
	
private:
	int32 TileIndex = INDEX_NONE;
	
	int32 DisplayStackCount = 0;
	
	int32 UpperLeftIndex = INDEX_NONE; // 左上角坐标，也是堆栈计数的索引
	
	TWeakObjectPtr<UInvInventoryItem> InventoryItem; // 观察是否装有物体
	
	bool bAvailable = true;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_GridSlot; // 网格背景图片
	
	EInvGridSlotState GridSlotState;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_Occupied;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_Unoccupied;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_Selected;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_GrayedOut;

	// 观察是否存在ItemPopUp，防止对于物品多次点击导致多次创建ItemPopUp
	TWeakObjectPtr<UInvItemPopUp> ItemPopUp;
};

