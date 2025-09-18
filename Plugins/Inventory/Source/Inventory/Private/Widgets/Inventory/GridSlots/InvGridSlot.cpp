// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/GridSlots/InvGridSlot.cpp

#include "Widgets/Inventory/GridSlots/InvGridSlot.h"

#include "Components/Image.h"
#include "Items/InvInventoryItem.h"
#include "Widgets/ItemPopUp/InvItemPopUp.h"

void UInvGridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	GridSLotHovered.Broadcast(TileIndex, InMouseEvent);
}

void UInvGridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	GridSLotUnHovered.Broadcast(TileIndex, InMouseEvent);
}

FReply UInvGridSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	GridSlotClicked.Broadcast(TileIndex, InMouseEvent);
	
	return FReply::Handled(); // FReply(bIsHandled = true) 
}

void UInvGridSlot::SetInventoryItem(UInvInventoryItem* Item)
{
	InventoryItem = Item;
}

void UInvGridSlot::SetOccupiedTexture()
{
	GridSlotState = EInvGridSlotState::Occupied;
	Image_GridSlot->SetBrush(Brush_Occupied);
}

void UInvGridSlot::SetUnOccupiedTexture()
{
	GridSlotState = EInvGridSlotState::Unoccupied;
	Image_GridSlot->SetBrush(Brush_Unoccupied);
}

void UInvGridSlot::SetSelectedTexture()
{
	GridSlotState = EInvGridSlotState::Selected;
	Image_GridSlot->SetBrush(Brush_Selected);
}

void UInvGridSlot::SetGrayedOutTexture()
{
	GridSlotState = EInvGridSlotState::GrayedOut;
	Image_GridSlot->SetBrush(Brush_GrayedOut);
}

void UInvGridSlot::SetItemPopUp(UInvItemPopUp* PopUp)
{
	this->ItemPopUp = PopUp;
	ItemPopUp->SetGridIndex(GetIndex());
	ItemPopUp->OnNativeDestruct.AddUObject(this, &UInvGridSlot::OnItemPopUpDestruct);
}

UInvItemPopUp* UInvGridSlot::GetItemPopUp() const
{
	return ItemPopUp.Get();
}

void UInvGridSlot::OnItemPopUpDestruct(UUserWidget* Menu)
{
	ItemPopUp.Reset(); // 销毁ItemPopUp时重置观察指针
}
