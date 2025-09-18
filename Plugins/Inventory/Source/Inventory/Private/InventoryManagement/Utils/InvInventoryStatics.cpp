// Fill out your copyright notice in the Description page of Project Settings.
// InventoryManagement/Utils/InvInventoryStatics.cpp

#include "InventoryManagement/Utils/InvInventoryStatics.h"

#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "Widgets/Inventory/InventoryBase/InvInventoryBase.h"

UInvInventoryComponent* UInvInventoryStatics::GetInvInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
		return nullptr;

	UInvInventoryComponent* InventoryComponent = PlayerController->FindComponentByClass<UInvInventoryComponent>();

	return InventoryComponent;
}

EInvItemCategory UInvInventoryStatics::GetItemCategoryFromItemComp(UInvItemComponent* ItemComp)
{
	if (!IsValid(ItemComp))
		return EInvItemCategory::None;

	return ItemComp->GetItemManifest().GetItemCategory();
}

void UInvInventoryStatics::ItemHovered(APlayerController* PC, UInvInventoryItem* Item)
{
	UInvInventoryComponent* IC = GetInvInventoryComponent(PC);
	if (!IsValid(IC))
		return;

	UInvInventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase))
		return;

	if (InventoryBase->HasHoverItem())
		return;

	InventoryBase->OnItemHovered(Item);
}

void UInvInventoryStatics::ItemUnhovered(APlayerController* PC)
{
	UInvInventoryComponent* IC = GetInvInventoryComponent(PC);
	if (!IsValid(IC))
		return;

	UInvInventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase))
		return;

	InventoryBase->OnItemUnhovered();
}

UInvHoverItem* UInvInventoryStatics::GetHoverItem(APlayerController* PC)
{
	UInvInventoryComponent* IC = GetInvInventoryComponent(PC);
	if (!IsValid(IC))
		return nullptr;

	UInvInventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase))
		return nullptr;

	return InventoryBase->GetHoverItem();
}

UInvInventoryBase* UInvInventoryStatics::GetInventoryWidget(APlayerController* PC)
{
	UInvInventoryComponent* IC = GetInvInventoryComponent(PC);
	if (!IsValid(IC))
		return nullptr;

	return IC->GetInventoryMenu();
}
