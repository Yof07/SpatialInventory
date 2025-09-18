// Fill out your copyright notice in the Description page of Project Settings.
// Items/Manifest/InvInventoryItem.cpp
#include "Items/InvInventoryItem.h"

#include "Net/UnrealNetwork.h"

void UInvInventoryItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps); // 确保处理父类的同步

	// 执行生命周期复制
	DOREPLIFETIME(ThisClass, ItemManifest); // 把本类要复制的属性注册到同步(复制)列表
	DOREPLIFETIME(ThisClass, TotalStackCount);
}

void UInvInventoryItem::SetItemManifest(const FInvItemManifest& Manifest)
{
	ItemManifest = FInstancedStruct::Make<FInvItemManifest>(Manifest);
}

bool UInvInventoryItem::IsStackable() const
{
	const FInvStackableFragment* Stackable = GetItemManifest().GetFragmentOfType<FInvStackableFragment>();
	return Stackable != nullptr;
}

bool UInvInventoryItem::IsConsumable() const
{
	return GetItemManifest().GetItemCategory() == EInvItemCategory::Consumable;
}
