// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/HUD/InvHUDWidget.cpp

#include "Widgets/HUD/InvHUDWidget.h"

#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "InventoryManagement/Utils/InvInventoryStatics.h"
#include "Widgets/HUD/InvInfoMessage.h"

void UInvHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UInvInventoryComponent* InventoryComponent = UInvInventoryStatics::GetInvInventoryComponent(GetOwningPlayer());

	if (!IsValid(InventoryComponent))
		return;

	InventoryComponent->NoRoomInInventory.AddDynamic(this, &ThisClass::OnNoRoom);
}

void UInvHUDWidget::OnNoRoom()
{
	if (!IsValid(InfoMessage))
		return;

	InfoMessage->SetMessage(InfoMessageText);
}
