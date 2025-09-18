// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/GridSlots/InvEquippedGridSlot.cpp

#include "Widgets/Inventory/GridSlots/InvEquippedGridSlot.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "InventoryManagement/Utils/InvInventoryStatics.h"
#include "Items/Fragments/InvFragmentTags.h"
#include "Widgets/Inventory/HoverItem/InvHoverItem.h"
#include "Widgets/Inventory/SlottedItems/InvEquippedSlottedItem.h"

void UInvEquippedGridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!IsAvailable())
		return;

	UInvHoverItem* HoverItem = UInvInventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem))
		return;

	if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag)) // 谁的tag更精确？
	{
		SetOccupiedTexture(); // 高亮
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInvEquippedGridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (!IsAvailable())
		return;

	UInvHoverItem* HoverItem = UInvInventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem))
		return;

	if (IsValid(EquippedSlottedItem)) // 防止在已经有装备的情况下，再次调用这个函数时把UnOccupied纹理设置上去
		return;

	if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag)) 
	{
		SetUnOccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

FReply UInvEquippedGridSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	EquippedGridSlotClicked.Broadcast(this, EquipmentTypeTag);
	return FReply::Handled();
}

UInvEquippedSlottedItem* UInvEquippedGridSlot::OnItemEquipped(UInvInventoryItem* Item, const FGameplayTag& EquipmentTag, float TileSize)
{
	// 检查装备类型标签
	if (!EquipmentTag.MatchesTagExact(EquipmentTypeTag))
		return nullptr;
	
	// 获取网格尺寸
	const FInvGridFragment* GridFragment = GetFragment<FInvGridFragment>(Item, FragmentTags::GridFragment);
	if (!GridFragment)
		return nullptr;
	const FIntPoint GridDimensions = GridFragment->GetGridSize();

	// 计算已装备槽位物品的绘制大小
	const float IconTileWith = TileSize - GridFragment->GetGridPadding() * 2; // 左右都有偏移
	const FVector2D DrawSize  = GridDimensions * IconTileWith;
	
	// 创建已装备槽位物品的 Widget
	EquippedSlottedItem = CreateWidget<UInvEquippedSlottedItem>(GetOwningPlayer(), EquippedSlottedItemClass);
	
	// 设置槽位物品对应的库存物品
	EquippedSlottedItem->SetInventoryItem(Item);
	
	// 设置槽位物品的装备类型标签
	EquippedSlottedItem->SetEquipmentTypeTag(EquipmentTag);
	
	// 隐藏槽位物品上的堆叠计数 Widget
	EquippedSlottedItem->UpdateStackCount(0);
	
	// 在此类（已装备网格槽）上设置库存物品
	SetInventoryItem(Item);
	
	// 为已装备槽位物品设置图像画刷
	const FInvImageFragment* ImageFragment = GetFragment<FInvImageFragment>(Item, FragmentTags::IconFragment);
	if (!ImageFragment)
		return nullptr;

	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Type::Image;
	Brush.ImageSize = DrawSize;

	EquippedSlottedItem->SetImageBrush(Brush);
	
	// 将槽位物品作为子项添加到该 Widget 的 Overlay 中
	Overlay_Root->AddChildToOverlay(EquippedSlottedItem);
	FGeometry OverlayGeometry = Overlay_Root->GetCachedGeometry();
	auto OverlayPos = OverlayGeometry.Position;
	auto OverlaySize = OverlayGeometry.Size;

	const float LeftPadding = OverlaySize.X / 2.f - DrawSize.X / 2.f;
	const float TopPadding = OverlaySize.Y / 2.f - DrawSize.Y / 2.f;

	UOverlaySlot* OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(EquippedSlottedItem);
	OverlaySlot->SetPadding(FMargin(LeftPadding, TopPadding));
	
	// 返回已装备槽位物品的 Widget
	return EquippedSlottedItem;
}
