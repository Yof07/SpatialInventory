// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/SlottedItems/InvSlottedItem.cpp

#include "Widgets/Inventory/SlottedItems/InvSlottedItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryManagement/Utils/InvInventoryStatics.h"

FReply UInvSlottedItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnSlottedItemClicked.Broadcast(GirdIndex, InMouseEvent);
	return FReply::Handled();
}

void UInvSlottedItem::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UInvInventoryStatics::ItemHovered(GetOwningPlayer(), InventoryItem.Get());
}

void UInvSlottedItem::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	UInvInventoryStatics::ItemUnhovered(GetOwningPlayer());
}

void UInvSlottedItem::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush); // 应用画刷来绘制自身
}

void UInvSlottedItem::UpdateStackCount(int32 StackCount)
{
	if (StackCount > 0)
	{
		Text_StackCount->SetText(FText::AsNumber(StackCount));
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}
