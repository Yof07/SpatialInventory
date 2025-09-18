

#include "Items/Manifest/InvItemManifest.h"

#include "Items/InvInventoryItem.h"
#include "Items/Components/InvItemComponent.h"
#include "Widgets/Composite/InvCompositeBase.h"


UInvInventoryItem* FInvItemManifest::Manifest(UObject* NewOuter)
{
	UInvInventoryItem* Item = NewObject<UInvInventoryItem>(NewOuter, UInvInventoryItem::StaticClass());

	// 将Item的Manifest设置为清单本身
	Item->SetItemManifest(*this);

	// 用以下方式来记住物品随机值(记住物品随机值：一个物品在游戏里时会生成一个从游戏开始到结束都不变的随机值，即使丢弃再拾取也是一样，随机是相对于每次开始游戏时而言的)
	for (auto& Fragment : Item->GetItemManifestMutable().GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}

	ClearFragments();
	
	return Item;
} 

void FInvItemManifest::AssimilateInventoryFragments(UInvCompositeBase* Composite) const
{
	// 遍历当前定义的所有片段
	const auto& InventoryItemFragments = GetAllFragmentsOfType<FInvInventoryItemFragment>();
	for (const auto* InventoryItemFragment : InventoryItemFragments)
	{
		/**
		 * 让Composite去描述每个片段的信息;
		 * 如果这个Composite是组合而非叶子，那么它会让所有叶子都执行ApplyFunction;
		 * 如果这个Composite是叶子而非组合，那么它就会执行传入的Function()
		 */ 
		Composite->ApplyFunction([InventoryItemFragment](UInvCompositeBase* Widget)
		{
			InventoryItemFragment->Assimilate(Widget); // InvLeaf.h 的 ApplyFunction()中的Function(this)
		});
	}
}

void FInvItemManifest::SpawnPickUpActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!IsValid(WorldContextObject) || !IsValid(PickUpActorClass))
		return;

	AActor* SpawnedActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickUpActorClass, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedActor))
		return;

	// 让生成的物体的ItemManifest和当前ItemManifest相同
	UInvItemComponent* ItemComp = SpawnedActor->FindComponentByClass<UInvItemComponent>();
	check(ItemComp);
	ItemComp->InitItemManifest(*this);
}

void FInvItemManifest::ClearFragments()
{
	for (auto& Fragment : Fragments)
	{
		Fragment.Reset();
	}

	Fragments.Empty();
}
