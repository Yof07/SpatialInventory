// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/Spatial/InvInventoryGrid.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/InvInventoryItem.h"
#include "Types/InvGridTypes.h"
#include "Widgets/Inventory/GridSlots/InvGridSlot.h"
#include "InvInventoryGrid.generated.h"

class UInvItemPopUp;
class UInvHoverItem;
class UInvSlottedItem;
class UInvItemComponent;
class UInvInventoryComponent;
class UCanvasPanel;
class UInvGridSlot;
enum class EInvGridSlotState : uint8;

/**
 * 背包网格
 */
UCLASS()
class INVENTORY_API UInvInventoryGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	EInvItemCategory GetItemCategory() const { return ItemCategory; }

	FInvSlotAvailabilityResult HasRoomForItem(const UInvItemComponent* ItemComponent);

	virtual void NativeOnInitialized() override;

	// UserWidget类中的虚函数
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	/**
	 * 绑定于InventoryComponent的OnItemAdded的回调
	 * @param Item 
	 */
	UFUNCTION()
	void AddItem(UInvInventoryItem* Item);

	void ShowCursor();
	void HideCursor();

	void SetOwningCanvas(UCanvasPanel* OwningCanvas);
	
	/**
	 * 调用DropItem的RPC
	 */
	void DropItem();

	bool HasHoverItem() const;

	UInvHoverItem* GetHoverItem() const;

	float GetTileSize() const { return TileSize; }

	void ClearHoverItem();

	/** 点击一个物品后它会悬浮并跟随鼠标移动 */
	void AssignHoverItem(UInvInventoryItem* InventoryItem);
	
private:
	/** UI更新 */
	UFUNCTION()
	void AddStacks(const FInvSlotAvailabilityResult& Result);

	// FSlottedItemClicked的回调
	UFUNCTION()
	void OnSlottedItemClicked(const int32 GridIndex, const FPointerEvent& MouseEvent);

	// 委托GridSlotClicked的回调
	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

	// 鼠标覆盖在gridslot时highlight
	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	// 鼠标离开gridslot时unhighlight
	UFUNCTION()
	void OnGridSlotUnHovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	/** 物品分割回调 */
	UFUNCTION()
	void OnPopUpMenuSplit(int32 SplitAmount, int32 Index);

	/** 物品丢弃回调 */
	UFUNCTION()
	void OnPopUpMenuDrop(int32 Index);

	/** 物品使用回调 */
	UFUNCTION()
	void OnPopUpMenuConsume(int32 Index);
	
	// 生成 Gridslots（背包网格）
	void ConstructGrid();

	/**
	 * 
	 * @param InventoryItem 
	 * @param GridIndex 
	 * @param PreviousGridIndex 
	 */
	void AssignHoverItem(UInvInventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousGridIndex);
	
	// 添加的物品类别是否等于Grid的类别
	bool MatchesCategory(const UInvInventoryItem* Item) const;
	
	FInvSlotAvailabilityResult HasRoomForItem(const UInvInventoryItem* Item);

	/** 根据物品的定义检查该物品是否能放入仓库中 */
	FInvSlotAvailabilityResult HasRoomForItem(const FInvItemManifest& Manifest);

	void AddItemToIndices(const FInvSlotAvailabilityResult& Result, UInvInventoryItem* NewItem); // Index->Indices

	FVector2D GetDrawSize(const FInvGridFragment* GridFragment) const;

	void SetSlottedItemImage(const UInvSlottedItem* SlottedItem, const FInvGridFragment* GridFragment, const FInvImageFragment* ImageFragment) const;

	void AddItemAtIndex(UInvInventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount);

	UInvSlottedItem* CreateSlottedItem(UInvInventoryItem* Item, const bool bStackable, const int32 StackAmount,
		const FInvGridFragment* GridFragment, const FInvImageFragment* ImageFragment, const int32 Index) const;

	void AddSlottedItemToCanvas(const int32 Index, const FInvGridFragment* GridFragment, UInvSlottedItem* SlottedItem) const;
	
	/**
	 *	更新指定矩形区域的GridSLot
	 * 
	 * @param NewItem 用来更新GridSlot中的InventoryItem字段
	 * @param Index 被更新的矩形区域的左上角的GridSLot的坐标
	 * @param bStackableItem 是否可堆叠
	 * @param StackAmount 如果可堆叠，则更新矩形区域左上角的GridSlot(即GridSlots[Index])的StackCount为StackAmount
	 */
	void UpdateGridSlots(UInvInventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount);

	bool IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const;
 
	bool HasRoomAtIndex(const UInvGridSlot* GridSlot, const FIntPoint& Dimensions, const TSet<int32>& CheckedIndices, TSet<int32>&
	                    OutTentativelyClaimed, const FGameplayTag& ItemType, const int32 MaxStackSize);

	// 针对一个槽位(GridSlot)进行槽约束检测
	bool CheckSlotConstraints(const UInvGridSlot* GridSlot, const UInvGridSlot* SubGridSlot, const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed, const
	                          FGameplayTag& ItemType, const int32 MaxStackSize) const;

	// 网格槽位是否有有效的物品
	bool HasValidItem(const UInvGridSlot* GridSlot) const;

	bool IsUpperLeftSlot(const UInvGridSlot* GridSlot, const UInvGridSlot* SubGridSlot) const;

	bool DoesItemTypeMatch(const UInvInventoryItem* SubItem, const FGameplayTag& ItemType) const;

	/**
	 * 
	 * @param StartIndex 
	 * @param ItemDimensions 指物体所占据的网格范围
	 * @return 
	 */
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const;

	int32 DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize, const int32 AmountToFill, const UInvGridSlot* GridSlot) const;

	int32 GetStackAmount(const UInvGridSlot* GridSlot) const;

	FIntPoint GetItemDimensions(const FInvItemManifest& Manifest) const;

	bool IsRightClick(const FPointerEvent& MouseEvent) const;
	bool IsLeftClick(const FPointerEvent& MouseEvent) const;

	void PickUp(UInvInventoryItem* ClickedInventoryItemItem, const int32 GridIndex);

	void RemoveItemFromGrid(UInvInventoryItem* InventoryItem, const int32 GridIndex);

	void UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition);

	FIntPoint CalculateHoveredCoordinates(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const;

	/** 计算鼠标在网格单元中的象限 */
	EInvTileQuadrant CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const;

	// FInvTileParameters更新时调用，HoverItem存在时才调用
	void OnTileParametersUpdated(const FInvTileParameters& Parameters);

	/**
	 * 以给定参考格子 Coordinate 为中心，计算放置一个尺寸为 Dimensions 的矩形物品时的起始（左上）格子坐标。当 Dimensions 为偶数时，
	 * 中点并不唯一，Quadrant 用来决定具体偏向哪一侧（左/右、上/下）。
	 * 
	 * @param Coordinate 
	 * @param Dimensions 
	 * @param Quadrant 
	 * @return 
	 */
	FIntPoint CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions, const EInvTileQuadrant Quadrant) const;

	/**
	 * 以HoverItem对GridSlots悬停的位置进行检查
	 * @param Position HoverItem的位置
	 * @param Dimensions HoverItem的矩形大小
	 * @return 
	 */
	FInvSpaceQueryResult CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions);

	/** 判断鼠标是否在CanvasPanel内并更新bLastMouseWithinCanvas和bMouseWithinCanvas */
	bool CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundarySize, const FVector2D& Location);

	void HighlightSlots(const int32 Index, const FIntPoint& Dimensions);
	void UnHighlightSlots(const int32 Index, const FIntPoint& Dimensions);

	void ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EInvGridSlotState GridSlotState);

	/** 把悬浮物品放置在指定Index */
	void PutDownOnIndex(const int32 Index);

	UUserWidget* GetVisibleCursorWidget();
	UUserWidget* GetHiddenCursorWidget();

	// 是同一物品且可叠加
	bool IsSameStackable(const UInvInventoryItem* ClickedInventoryItem) const;

	/** 悬浮物品和网格物品原封不动交换 */
	void SwapWithHoverItem(UInvInventoryItem* ClickedInventoryItem, const int32 GridIndex);

	bool ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount, const int32 MaxStackSize) const;

	void SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index);

	// RoomInClickedSlot指距离堆叠满还可以填充的数量
	bool ShouldConsumeHoverItemStacks(const int32 HoveredStackCount, const int32 RoomInClickedSlot) const;
	
	void ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index);

	bool ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const;

	void FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index);

	void CreateItemPopUp(const int32 GridIndex);

	/*** Data Members ***/
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	EInvItemCategory ItemCategory;

	UPROPERTY() 
	TArray<TObjectPtr<UInvGridSlot>> GridSlots;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInvGridSlot> GridSlotClass;

	/** 将GirdSlot和SlottedItem纳为CanvasPanel的子控件并绘制 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel; 

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInvSlottedItem> SlottedItemClass;

	TMap<int32, TObjectPtr<UInvSlottedItem>> SlottedItems;

	UPROPERTY()
	TWeakObjectPtr<UInvInventoryComponent> InventoryComponent;

	/** 行 */
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Rows;

	/** 列 */
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Columns;

	/** 每个格子的宽高 */
	UPROPERTY(EditAnywhere, Category="Inventory")
	float TileSize;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInvHoverItem> HoverItemClass;

	UPROPERTY()
	TObjectPtr<UInvHoverItem> HoverItem;

	FInvTileParameters LastTileParameters; // 上一帧的瓦片参数
	FInvTileParameters TileParameters;

	// 如果我们在有效位置点击网格时物品应该放置的索引
	int32 ItemDropIndex = INDEX_NONE;

	FInvSpaceQueryResult CurrentQueryResult;

	bool bLastMouseWithinCanvas;
	bool bMouseWithinCanvas; // 鼠标当前是否在canvas内

	int32 LastHighlightedIndex;
	FIntPoint LastHighlightedDimensions;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UUserWidget> VisibleCursorWidgetClass;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UUserWidget> HiddenCursorWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> VisibleCursorWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> HiddenCursorWidget;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInvItemPopUp> ItemPopUpClass;

	UPROPERTY()
	TObjectPtr<UInvItemPopUp> ItemPopUp;

	TWeakObjectPtr<UCanvasPanel> OwningCanvasPanel;

	// ItemPopUp显示位置偏移
	UPROPERTY(EditAnywhere, Category="Inventory")
	FVector2D ItemPopUpOffset;
};



