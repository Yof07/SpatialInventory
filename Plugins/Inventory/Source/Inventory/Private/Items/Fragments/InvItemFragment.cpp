// Items/Fragments/InvItemFragment.cpp


#include "Items/Fragments/InvItemFragment.h"

#include "EquipmentManagement/EquipActor/InvEquipActor.h"
#include "Widgets/Composite/InvCompositeBase.h"
#include "Widgets/Composite/InvLeaf_Image.h"
#include "Widgets/Composite/InvLeaf_LabeledValue.h"
#include "Widgets/Composite/InvLeaf_Text.h"

void FInvInventoryItemFragment::Assimilate(UInvCompositeBase* Composite) const
{
	if (!MatchesWidgetTag(Composite))
		return;

	// 展示面板
	Composite->Expand();
}

bool FInvInventoryItemFragment::MatchesWidgetTag(const UInvCompositeBase* Composite) const
{
	return Composite->GetFragmentTag().MatchesTagExact(GetFragmentTag());
}

void FInvImageFragment::Assimilate(UInvCompositeBase* Composite) const
{
	FInvInventoryItemFragment::Assimilate(Composite);

	if (!MatchesWidgetTag(Composite))
		return;

	UInvLeaf_Image* Image = Cast<UInvLeaf_Image>(Composite);
	if (!IsValid(Image))
		return;

	Image->SetImage(Icon);
	Image->SetBoxSize(IconDimensions);
	Image->SetBoxSize(IconDimensions);
}

void FInvTextFragment::Assimilate(UInvCompositeBase* Composite) const
{
	FInvInventoryItemFragment::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite))
		return;

	UInvLeaf_Text* LeafText = Cast<UInvLeaf_Text>(Composite);
	if (!IsValid(LeafText))
		return;

	LeafText->SetText(FragmentText);
}

void FInvLabeledNumberFragment::Manifest()
{
	FInvInventoryItemFragment::Manifest();

	if (bRandomizeOnManifest)
	{
		Value = FMath::RandRange(Min, Max);
	}

	bRandomizeOnManifest = false; // 这样即使再次实例化，也不会生成新的随机值
}

void FInvLabeledNumberFragment::Assimilate(UInvCompositeBase* Composite) const
{
	FInvInventoryItemFragment::Assimilate(Composite);

	if (!MatchesWidgetTag(Composite))
		return;

	UInvLeaf_LabeledValue* LabeledValue = Cast<UInvLeaf_LabeledValue>(Composite);
	if (!IsValid(LabeledValue))
		return;
	
	LabeledValue->SetText_Label(Text_Label, bCollapseLabel);

	FNumberFormattingOptions Options; // 数字显示格式
	Options.MinimumFractionalDigits = MinFractionalDigits;
	Options.MaximumFractionalDigits = MaxFractionalDigits;
	LabeledValue->SetText_Value(FText::AsNumber(Value,&Options), bCollapseValue);
}

void FInvConsumableFragment::OnConsume(APlayerController* PC)
{
	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnConsume(PC);
	}
}

void FInvConsumableFragment::Assimilate(UInvCompositeBase* Composite) const
{
	FInvInventoryItemFragment::Assimilate(Composite);
	for (const auto& Modifier : ConsumeModifiers)
	{
		const auto& ModRef = Modifier.Get();
		ModRef.Assimilate(Composite);
	}
}

void FInvConsumableFragment::Manifest()
{
	FInvInventoryItemFragment::Manifest();
	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.Manifest();
	}
}

void FInvHealthPotionFragment::OnConsume(APlayerController* PC)
{
	// 从玩家控制器（PC）或 PC->GetPawn() 获取一个统计组件（Stats Component）
	// 或者获取能力系统组件（Ability System Component）并应用一个游戏效果（Gameplay Effect）
	// 或者调用接口函数来执行治疗（Healing）
	
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Green,
		FString::Printf(TEXT("消耗生命药水，补充血量： %f"),GetValue()),
		false);
}

void FInvManaPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("消耗魔法药水，补充蓝量： %f"), GetValue()), false);
}

void FInvStrengthModifier::OnEquip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Green,
		FString::Printf(TEXT("穿戴装备。力量增加： %f"),GetValue()),
		false);
}

void FInvStrengthModifier::OnUnequip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Red,
		FString::Printf(TEXT("卸下装备。力量减少： %f"),GetValue()),
		false);
}

void FInvEquipmentFragment::OnEquip(APlayerController* PC)
{
	if (bEquipped)
		return;

	bEquipped = true;
	
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnEquip(PC); 
	}
}

void FInvEquipmentFragment::OnUnequip(APlayerController* PC)
{
	if (!bEquipped)
		return;

	bEquipped = false;
	
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnUnequip(PC); 
	}
}

void FInvEquipmentFragment::Assimilate(UInvCompositeBase* Composite) const
{
	FInvInventoryItemFragment::Assimilate(Composite);
	for (const auto& Modifier : EquipModifiers)
	{
		const auto& ModRef = Modifier.Get();
		ModRef.Assimilate(Composite);
	}
}

void FInvEquipmentFragment::Manifest()
{
	FInvInventoryItemFragment::Manifest();
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.Manifest(); // 设置修饰值？
	}
}

AInvEquipActor* FInvEquipmentFragment::SpawnAttachedActor(USkeletalMeshComponent* AttachMesh, bool bIsProxyMesh) const
{
	if (!IsValid(EquipActorClass) || !IsValid(AttachMesh))
		return nullptr;
	
	AInvEquipActor* SpawnedActor = AttachMesh->GetWorld()->SpawnActor<AInvEquipActor>(EquipActorClass);
	if (bIsProxyMesh)
	{
		SpawnedActor->SetReplicates(false);
		AttachMesh->SetIsReplicated(false);
	}
	SpawnedActor->AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketAttachPoint);

	UE_LOG(LogTemp, Warning, TEXT("SpawnedActor is replicated or not : %d"), SpawnedActor->GetIsReplicated());
	
	return SpawnedActor;
}

void FInvEquipmentFragment::DestroyAttachedActor() const
{
	if (EquippedActor.IsValid())
	{
		EquippedActor->Destroy();
	}
}

void FInvEquipmentFragment::SetEquippedActor(AInvEquipActor* EquipActor)
{
	EquippedActor = EquipActor;
}















