// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/HoverItem/InvHoverItem.cpp

#include "Widgets/Inventory/HoverItem/InvHoverItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/InvInventoryItem.h"

void UInvHoverItem::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush);
}

void UInvHoverItem::UpdateStackCount(const int32 Count)
{
	StackCount = Count; // 解决移动物品时stackcount隐藏的问题
	
	if (Count > 0)
	{
		Text_StackCount->SetText(FText::AsNumber(Count));
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}

FGameplayTag UInvHoverItem::GetItemType() const
{
	if (!InventoryItem.IsValid())
		return FGameplayTag::EmptyTag;

	return InventoryItem->GetItemManifest().GetItemType();
}

void UInvHoverItem::SetIsStackable(const bool bStacks)
{
	bIsStackable = bStacks;
	if (!bStacks)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UInvInventoryItem* UInvHoverItem::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UInvHoverItem::SetInventoryItem(UInvInventoryItem* Item)
{
	InventoryItem = Item;
}
