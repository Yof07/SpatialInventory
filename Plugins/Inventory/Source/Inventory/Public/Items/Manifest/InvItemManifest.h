#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Items/Fragments/InvItemFragment.h"
#include "Types/InvGridTypes.h"
#include "StructUtils/InstancedStruct.h" // 引用InvItemManifest.h这个头文件一般也会实例化这个结构体，所以也引用这个头文件，还要添加模块依赖
#include "InvItemManifest.generated.h"

class UInvCompositeBase;
/** 
 * 物品清单
 * 包含创建InventoryItem所需要的信息
 */
USTRUCT(BlueprintType)
struct INVENTORY_API FInvItemManifest // 外部模块使用，加上宏
{
	GENERATED_BODY()

	UInvInventoryItem* Manifest(UObject* NewOuter); // Outer即拥有者，负责管理所创建对象的生命周期
	
	EInvItemCategory GetItemCategory() const { return ItemCategory; }

	FGameplayTag GetItemType() const { return ItemType; }

	void AssimilateInventoryFragments(UInvCompositeBase* Composite) const;

	// 类型相同 + 标签匹配
	template<typename T> requires std::derived_from<T, FInvItemFragment>
	const T* GetFragmentOfTypeWithTag(const FGameplayTag& FragmentTag) const;

	// 类型相同即可
	template<typename T> requires std::derived_from<T, FInvItemFragment>
	const T* GetFragmentOfType() const;
	
	template<typename T> requires std::derived_from<T, FInvItemFragment>
	T* GetFragmentOfTypeMutable();
	
	template<typename T> requires std::derived_from<T, FInvItemFragment>
	TArray<const T*> GetAllFragmentsOfType() const;
	
	/** FInvItemManifest不继承自UObject，而通过UObject可以获取UWorld来动态生成Actor，这就是为什么需要额外获取WorldContextObject */
	void SpawnPickUpActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation);
	
	TArray<TInstancedStruct<FInvItemFragment>>& GetFragmentsMutable() { return Fragments; } 
	
private:
	void ClearFragments();
	
	/**** Data Members ****/
private:
	/** 我们只能添加FInvItemFragment的子类到数组中，排除FInvItemFragment类本身 */
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (ExcludeBaseStruct)) 
	TArray<TInstancedStruct<FInvItemFragment>> Fragments;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	EInvItemCategory ItemCategory = EInvItemCategory::None;
	
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (Categories = "GameItems"))
	FGameplayTag ItemType;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<AActor> PickUpActorClass;
};

template <typename T> requires std::derived_from<T, FInvItemFragment>
const T* FInvItemManifest::GetFragmentOfTypeWithTag(const FGameplayTag& FragmentTag) const
{
	// 遍历片段，并判断该片段与传入的片段的片段Tag是否匹配
	for (const TInstancedStruct<FInvItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			if (!FragmentPtr->GetFragmentTag().MatchesTag(FragmentTag))
				continue;
			
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInvItemFragment>
const T* FInvItemManifest::GetFragmentOfType() const
{
	// 遍历片段，并判断该片段的类型与要求的类型T是否相等
	for (const TInstancedStruct<FInvItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInvItemFragment>
T* FInvItemManifest::GetFragmentOfTypeMutable()
{
	for (TInstancedStruct<FInvItemFragment>& Fragment : Fragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInvItemFragment>
TArray<const T*> FInvItemManifest::GetAllFragmentsOfType() const
{
	TArray<const T*> Result;
	for (const TInstancedStruct<FInvItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			Result.Add(FragmentPtr);
		}
	}

	return Result;
}

