// InventoryManagement/FastArray/InvFastArray.h

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InvFastArray.generated.h"

struct FGameplayTag;
class UInvItemComponent;
class UInvInventoryComponent;
class UInvInventoryItem;

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FInvInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInvInventoryEntry() {}

private:
	friend struct FInvInventoryFastArray;
	
	friend UInvInventoryComponent;
	
	UPROPERTY()
	TObjectPtr<UInvInventoryItem> Item = nullptr;
	
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FInvInventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

	FInvInventoryFastArray() : OwnerComponent(nullptr) {}
	FInvInventoryFastArray(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	TArray<UInvInventoryItem*> GetAllItems() const;
	
	// FFastArraySerializer contract
	
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	
	// End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInvInventoryEntry, FInvInventoryFastArray>(Entries, DeltaParams, *this);
	}

	UInvInventoryItem* AddEntry(UInvItemComponent* ItemComponent);
	UInvInventoryItem* AddEntry(UInvInventoryItem* Item);

	void RemoveEntry(UInvInventoryItem* Item);

	UInvInventoryItem* FindFirstItemByType(const FGameplayTag& ItemType);
	
private:
	friend UInvInventoryComponent;
	
	// Replicated list of items
	UPROPERTY()
	TArray<FInvInventoryEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FInvInventoryFastArray> : public TStructOpsTypeTraitsBase2<FInvInventoryFastArray>
{
	enum  { WithNetDeltaSerializer = true };
};

