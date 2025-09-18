// InventoryManagement/FastArray/InvFastArray.cpp

#include "InventoryManagement/FastArray/InvFastArray.h"

#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "Items/InvInventoryItem.h"
#include "Items/Components/InvItemComponent.h"

TArray<UInvInventoryItem*> FInvInventoryFastArray::GetAllItems() const
{
	TArray<UInvInventoryItem*> Results;
	Results.Reserve(Entries.Num());
	
	for (const auto& Entry : Entries)
	{
		if (!IsValid(Entry.Item))
			continue;
		
		Results.Add(Entry.Item.Get());
	}

	return Results;
}

void FInvInventoryFastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UInvInventoryComponent* IC = Cast<UInvInventoryComponent>(OwnerComponent);

	if (!IsValid(IC))
		return;

	for (int32 Index : RemovedIndices)
	{
		IC->OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FInvInventoryFastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UInvInventoryComponent* IC = Cast<UInvInventoryComponent>(OwnerComponent);
	if (!IsValid(IC))
		return;

	for (int32 Index : AddedIndices)
	{
		IC->OnItemAdded.Broadcast(Entries[Index].Item); 
	}
}

UInvInventoryItem* FInvInventoryFastArray::AddEntry(UInvItemComponent* ItemComponent)
{
	check(OwnerComponent); //

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	UInvInventoryComponent* IC = Cast<UInvInventoryComponent>(OwnerComponent);
	if (!IsValid(IC))
		return nullptr;

	FInvInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = ItemComponent->GetItemManifest().Manifest(OwningActor); // 通过ItemComponent用Item清单来创建

	IC->AddRepSubObj(NewEntry.Item);
	MarkItemDirty(NewEntry);
	
	return NewEntry.Item.Get();
}

UInvInventoryItem* FInvInventoryFastArray::AddEntry(UInvInventoryItem* Item)
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner(); // PlayerController
	check(OwningActor->HasAuthority()); // ？？？

	FInvInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef(); // 在数组中添加元素并返回其引用
	NewEntry.Item = Item;
	
	MarkItemDirty(NewEntry);
	return Item;
}

void FInvInventoryFastArray::RemoveEntry(UInvInventoryItem* Item)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInvInventoryEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

UInvInventoryItem* FInvInventoryFastArray::FindFirstItemByType(const FGameplayTag& ItemType)
{
	auto FoundItem = Entries.FindByPredicate([ItemType = ItemType](const FInvInventoryEntry& Entry)
	{
		return IsValid(Entry.Item) && Entry.Item->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
	});

	return FoundItem ? FoundItem->Item.Get() : nullptr;
}
