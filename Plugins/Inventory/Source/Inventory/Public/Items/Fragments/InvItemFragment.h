// Items/Fragments/InvItemFragment.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "InvItemFragment.generated.h"


class AInvEquipActor;
class UInvCompositeBase;
/** FInvItemFragment */
USTRUCT(BlueprintType)
struct FInvItemFragment
{
	GENERATED_BODY()
	
	FInvItemFragment() {}
	// c++ rules of five
	FInvItemFragment(const FInvItemFragment&) = default;
	FInvItemFragment& operator=(const FInvItemFragment&) = default;
	FInvItemFragment(FInvItemFragment&&) = default;
	FInvItemFragment& operator=(FInvItemFragment&&) = default;
	virtual ~FInvItemFragment() {} // 自定义析构
	
	const FGameplayTag& GetFragmentTag() const { return FragmentTag; };
	void SetFragmentTag(const FGameplayTag& Tag) { FragmentTag = Tag; };

	virtual void Manifest() {}
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (Categories = "FragmentTags"))
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};

/** FInvInventoryItemFragment */
USTRUCT(BlueprintType)
struct FInvInventoryItemFragment : public FInvItemFragment
{
	GENERATED_BODY()

	virtual void Assimilate(UInvCompositeBase* Composite) const;

protected:
	bool MatchesWidgetTag(const UInvCompositeBase* Composite) const;
};

/** FInvGridFragment */
USTRUCT(BlueprintType)
struct FInvGridFragment : public FInvItemFragment
{	
	GENERATED_BODY()
	
	const FIntPoint& GetGridSize() const { return GridSize; };
	void SetGridSize(const FIntPoint& Size) { GridSize = Size; };
	
	float GetGridPadding() const { return GridPadding; };
	void SetGridPadding(float Padding) { GridPadding = Padding; };
		
private:
	// 占据格子空间，默认 1 × 1
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	FIntPoint GridSize = {1,1};

	// 图标边距
	UPROPERTY(EditAnywhere, Category="Inventory")
	float GridPadding = 0.f;
};


/** FInvImageFragment */
USTRUCT(BlueprintType)
struct FInvImageFragment : public FInvInventoryItemFragment
{
	GENERATED_BODY()

	UTexture2D* GetIcon() const { return Icon; };
	virtual void Assimilate(UInvCompositeBase* Composite) const override;
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FVector2D IconDimensions = {44.f, 44.f};
};

/** FInvTextFragment */
USTRUCT(BlueprintType)
struct FInvTextFragment : public FInvInventoryItemFragment
{
	GENERATED_BODY()

	FText GetText() const { return FragmentText; }
	void SetText(const FText& Text) { FragmentText = Text; };

	virtual void Assimilate(UInvCompositeBase* Composite) const override;
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FText FragmentText;
};

/** FInvTextFragment */
USTRUCT(BlueprintType)
struct FInvLabeledNumberFragment : public FInvInventoryItemFragment
{
	GENERATED_BODY()
	
	virtual void Manifest() override;

	virtual void Assimilate(UInvCompositeBase* Composite) const override;

	float GetValue() const { return Value; };

public:
	// 当首次生成该碎片时，它会进行随机化。但一旦装备并丢弃，物品应保留相同的数值，因此不应再次随机化。
	bool bRandomizeOnManifest {true};
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FText Text_Label {};

	UPROPERTY(VisibleAnywhere, Category="Inventory")
	float Value {0.f};

	UPROPERTY(EditAnywhere, Category="Inventory")
	float Max {0.f};

	UPROPERTY(EditAnywhere, Category="Inventory")
	float Min {0.f};
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	bool bCollapseLabel{false};
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	bool bCollapseValue{false};
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MinFractionalDigits {1};
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MaxFractionalDigits {1};
};

/** FInvStackableFragment */
USTRUCT(BlueprintType)
struct FInvStackableFragment : public FInvItemFragment
{
	GENERATED_BODY()
	
	int32 GetMaxStackSize() const { return MaxStackSize; };
	
	int32 GetStackCount() const { return StackCount; };
	void SetStackCount(const int32 Count) { StackCount = Count; };
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int32 MaxStackSize = 1; // 最大堆叠数量

	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 StackCount = 1; // 拾取该物体时增加的数量
};

// Consume Fragments

USTRUCT(BlueprintType)
struct FInvConsumeModifier : public FInvLabeledNumberFragment
{
	GENERATED_BODY()

	virtual void OnConsume(APlayerController* PC) {}
};

/** FInvConsumableFragment */
USTRUCT(BlueprintType)
struct FInvConsumableFragment : public FInvInventoryItemFragment
{
	GENERATED_BODY()

	/** 使用消耗品的效果 */
	virtual void OnConsume(APlayerController* PC);
	virtual void Assimilate(UInvCompositeBase* Composite) const override;
	virtual void Manifest() override;

private:
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInvConsumeModifier>> ConsumeModifiers;
};

/** FInvHealthPotionFragment */
USTRUCT(BlueprintType)
struct FInvHealthPotionFragment : public FInvConsumeModifier
{
	GENERATED_BODY()
	
	/** 生命药水治疗效果 */
	virtual void OnConsume(APlayerController* PC) override;
	
};

/** FInvManaPotionFragment */
USTRUCT(BlueprintType)
struct FInvManaPotionFragment : public FInvConsumeModifier
{
	GENERATED_BODY()
	
	/** 魔法药水治疗效果 */
	virtual void OnConsume(APlayerController* PC) override;
};

// Equipment Fragment

USTRUCT(BlueprintType)	
struct FInvEquipModifier : public FInvLabeledNumberFragment
{
	GENERATED_BODY()

public:
	virtual void OnEquip(APlayerController* PC) {}
	virtual void OnUnequip(APlayerController* PC) {}
};

USTRUCT(BlueprintType)	
struct FInvStrengthModifier : public FInvEquipModifier
{
	GENERATED_BODY()

public:
	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;
};


USTRUCT(BlueprintType)
struct FInvEquipmentFragment : public FInvInventoryItemFragment
{
	GENERATED_BODY()

	void OnEquip(APlayerController* PC);
	void OnUnequip(APlayerController* PC);

	virtual void Assimilate(UInvCompositeBase* Composite) const override;
	virtual void Manifest() override;

	/** 在给定的骨骼网格上生成一个装备 actor，并把它附着到指定 socket 上，最后返回该 actor 指针 */
	AInvEquipActor* SpawnAttachedActor(USkeletalMeshComponent* AttachMesh, bool bIsProxyMesh) const;
	void DestroyAttachedActor() const;
	FGameplayTag GetEquipmentType() const { return EquipmentType; }

	void SetEquippedActor(AInvEquipActor* EquipActor);

	TSubclassOf<AInvEquipActor> GetEquipActorClass() const { return EquipActorClass; }
	
public:
	bool bEquipped {false};
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	TArray<TInstancedStruct<FInvEquipModifier>> EquipModifiers;
	
	UPROPERTY(EditAnywhere, Category="Inventory") 
	FName SocketAttachPoint = NAME_None;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag EquipmentType = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<AInvEquipActor> EquipActorClass = nullptr;

	TWeakObjectPtr<AInvEquipActor> EquippedActor = nullptr;

	
};



