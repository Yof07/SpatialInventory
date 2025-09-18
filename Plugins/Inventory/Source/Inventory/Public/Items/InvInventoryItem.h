// Fill out your copyright notice in the Description page of Project Settings.
// Items/Manifest/InvInventoryItem.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Manifest/InvItemManifest.h"
#include "InvInventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInvInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 获取生命周期要复制的属性
	 * UObject的一个虚函数，收集所有服务端和客户端同步(复制)的属性
	 * @param OutLifetimeProps 
	 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool IsSupportedForNetworking() const override {  return true;  } // 这样这个项目才可以被添加到注册的子对象列表中
	
	void SetItemManifest(const FInvItemManifest& Manifest);
	const FInvItemManifest& GetItemManifest() const { return ItemManifest.Get<FInvItemManifest>(); }
	FInvItemManifest& GetItemManifestMutable() { return ItemManifest.GetMutable<FInvItemManifest>(); }

	// 是否有堆叠片段
	bool IsStackable() const;

	bool IsConsumable() const;

	int32 GetTotalStackCount() const { return TotalStackCount; };
	void SetTotalStackCount(const int32 StackCount) { TotalStackCount = StackCount; };
	
private:
	/**
	 * ItemManifest实例化类型只能是InvItemManifest或它的子类，且在编辑面板中(如果可编辑)，界面也只能选择该类型及其派生类
	 * Scripts表示它是一个脚本化USTRUCT(即 C++／UHT 生成的类型)
	 * Inventory 是它所在的模块（也是插件名）
	 * InvItemManifest 是结构体名
	 */
	UPROPERTY(VisibleAnywhere, meta = (BaseStruct = "/Scripts/Inventory.InvItemManifest"), Replicated)
	FInstancedStruct ItemManifest;
	
	/** 物品数量 */
	UPROPERTY(Replicated)
	int32 TotalStackCount = 0;
};

/**
 * 这个函数能是模板函数是因为TInstancedStruct<FInvItemFragment>本身就是模板
 * @tparam FragmentType 
 * @param Item 
 * @param Tag 根据Tag判断要返回的Fragment
 * @return 
 */
template <typename FragmentType>
const FragmentType* GetFragment(const UInvInventoryItem* Item, const FGameplayTag& Tag)
{
	if (!IsValid(Item))
		return nullptr; 	
	
	const FInvItemManifest& Manifest = Item->GetItemManifest();
	
	return Manifest.GetFragmentOfTypeWithTag<FragmentType>(Tag);
}





