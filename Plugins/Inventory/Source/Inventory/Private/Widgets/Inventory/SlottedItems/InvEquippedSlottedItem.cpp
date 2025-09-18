// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/SlottedItems/InvEquippedSlottedItem.cpp

#include "Widgets/Inventory/SlottedItems/InvEquippedSlottedItem.h"

FReply UInvEquippedSlottedItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnEquippedSlottedItemClicked.Broadcast(this);
	return FReply::Handled(); 
}
