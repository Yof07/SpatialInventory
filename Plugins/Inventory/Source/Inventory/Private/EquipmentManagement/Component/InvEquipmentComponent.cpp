// Fill out your copyright notice in the Description page of Project Settings.
// EquipmentComponent/Component/InvEquipmentComponent.cpp

#include "EquipmentManagement/Component/InvEquipmentComponent.h"

#include "EquipmentManagement/EquipActor/InvEquipActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "InventoryManagement/Utils/InvInventoryStatics.h"
#include "UObject/UObjectGlobals.h"

// Called when the game starts
void UInvEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	InitPlayerController();
}

void UInvEquipmentComponent::SetOwningSkeletalMesh(USkeletalMeshComponent* OwningMesh)
{
	OwningSkeletalMesh = OwningMesh;
}

void UInvEquipmentComponent::InitializeOwner(APlayerController* PlayerController)
{
	if (IsValid(PlayerController))
	{
		OwningPlayerController = PlayerController;
	}
	InitInventoryComponent();
}

void UInvEquipmentComponent::InitInventoryComponent()
{
	InventoryComponent = UInvInventoryStatics::GetInvInventoryComponent(OwningPlayerController.Get());
	if (!InventoryComponent.IsValid())
		return;
	
	if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &ThisClass::OnItemEquipped))
		InventoryComponent->OnItemEquipped.AddDynamic(this, &ThisClass::OnItemEquipped);
	
	if (!InventoryComponent->OnItemUnequipped.IsAlreadyBound(this, &ThisClass::OnItemUnequipped))
		InventoryComponent->OnItemUnequipped.AddDynamic(this, &ThisClass::OnItemUnequipped);
	
	if (!InventoryComponent->OnItemEquipped_OwnerView.IsAlreadyBound(this, &ThisClass::OnItemEquipped))
		InventoryComponent->OnItemEquipped_OwnerView.AddDynamic(this, &ThisClass::OnItemEquipped);
	
	if (!InventoryComponent->OnItemUnequipped_OwnerView.IsAlreadyBound(this, &ThisClass::OnItemUnequipped))
		InventoryComponent->OnItemUnequipped_OwnerView.AddDynamic(this, &ThisClass::OnItemUnequipped);
}

void UInvEquipmentComponent::InitPlayerController()
{
	// 确保是PC
	if (OwningPlayerController = Cast<APlayerController>(GetOwner()); OwningPlayerController.IsValid())
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn()); IsValid(OwnerCharacter))
		{
			OnPossessedPawnChange(nullptr, OwnerCharacter);
		}
		else
		{
			OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChange); // 如果没有possess character，则先绑定回调
		}
	}
}

void UInvEquipmentComponent::OnPossessedPawnChange(APawn* OldPawn, APawn* NewPawn)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn()); IsValid(OwnerCharacter))
	{
		OwningSkeletalMesh = OwnerCharacter->GetMesh();
	}

	InitInventoryComponent();
}

AInvEquipActor* UInvEquipmentComponent::SpawnEquippedActor(FInvEquipmentFragment* EquipmentFragment,
                                                           const FInvItemManifest& Manifest, USkeletalMeshComponent* AttachMesh)
{
	AInvEquipActor* SpawnedEquipActor = EquipmentFragment->SpawnAttachedActor(AttachMesh, bIsProxy); // 生成、附着并返回EquipActor
	SpawnedEquipActor->SetEquipmentType(EquipmentFragment->GetEquipmentType());
	SpawnedEquipActor->SetOwner(GetOwner());
	EquipmentFragment->SetEquippedActor(SpawnedEquipActor);
	return SpawnedEquipActor;
}

AInvEquipActor* UInvEquipmentComponent::FindEquippedActor(const FGameplayTag& EquipmentTypeTag)
{
	auto FoundActor = EquippedActors.FindByPredicate([&EquipmentTypeTag](const AInvEquipActor* EquippedActor)
	{
		return EquippedActor->GetEquipmentType().MatchesTagExact(EquipmentTypeTag);
	});

	return FoundActor ? *FoundActor : nullptr;
}

void UInvEquipmentComponent::OnItemEquipped(UInvInventoryItem* EquippedItem)
{
	if (!IsValid(EquippedItem))
		return;

	// 获取 fragment
	FInvItemManifest& ItemManifest = EquippedItem->GetItemManifestMutable();
	FInvEquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentOfTypeMutable<FInvEquipmentFragment>();
	if (!EquipmentFragment)
	{
		return;
	}

	// 需要 attach 的骨骼组件
	if (!OwningSkeletalMesh.IsValid())
	{
		return;
	}

	// ----- Proxy 路径（owner-view preview）: 仅在本地客户端执行，spawn non-replicated actor -----
	if (bIsProxy)
	{
		// 只在本地 controller 下执行 preview
		if (!OwningPlayerController.IsValid() || !OwningPlayerController->IsLocalController())
		{
			return;
		}

		// Spawn preview actor（SpawnEquippedActor 会基于 bIsProxy 参数进行 attach）
		AInvEquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());
		if (!IsValid(SpawnedEquipActor))
			return;

		// 强制为 non-replicated（以防 fragment 内部没有处理）
		SpawnedEquipActor->SetReplicates(false);
		SpawnedEquipActor->SetReplicateMovement(false);

		EquippedActors.Add(SpawnedEquipActor);
		return;
	}

	// ----- 非 Proxy 路径（server-authoritative）: 仅在服务器 authority 上执行，spawn replicated actor -----
	AActor* CompOwner = GetOwner();
	if (!IsValid(CompOwner) || !CompOwner->HasAuthority())
	{
		return;
	}

	// server side: 执行装备带来的游戏逻辑效果（如果 fragment 提供）
	EquipmentFragment->OnEquip(OwningPlayerController.Get());

	// Spawn replicated equip actor 并附加
	AInvEquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());
	if (!IsValid(SpawnedEquipActor))
		return;

	// 确保 replicated
	SpawnedEquipActor->SetReplicates(true);

	EquippedActors.Add(SpawnedEquipActor);
}

void UInvEquipmentComponent::OnItemUnequipped(UInvInventoryItem* UnequippedItem)
{
	if (!IsValid(UnequippedItem))
		return;

	// 获取 fragment
	FInvItemManifest& ItemManifest = UnequippedItem->GetItemManifestMutable();
	FInvEquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentOfTypeMutable<FInvEquipmentFragment>();
	if (!EquipmentFragment)
		return;

	// ----- Proxy 路径（owner-view preview）: 仅在本地客户端执行，销毁 non-replicated preview actor -----
	if (bIsProxy)
	{
		// 只在本地 controller 下执行 preview 的移除
		if (!OwningPlayerController.IsValid() || !OwningPlayerController->IsLocalController())
		{
			return;
		}

		// 从本地缓存中移除并销毁（RemoveEquippedActor 会 detach + destroy 并从 EquippedActors 中移除）
		RemoveEquippedActor(EquipmentFragment->GetEquipmentType());

		// 额外确保 fragment 内部保存的引用也被清理
		EquipmentFragment->SetEquippedActor(nullptr);
		EquipmentFragment->DestroyAttachedActor(); // 如果 fragment 内部还有残留 actor，尝试销毁（DestroyAttachedActor 会做 IsValid 判断）

		return;
	}

	// ----- 非 Proxy 路径（server-authoritative）: 仅在服务器 authority 上执行 -----
	AActor* CompOwner = GetOwner();
	if (!IsValid(CompOwner) || !CompOwner->HasAuthority())
	{
		return;
	}

	// 执行卸下带来的游戏逻辑（如果 fragment 提供）
	EquipmentFragment->OnUnequip(OwningPlayerController.Get());

	// 移除并销毁已附加的 replicated equip actor
	RemoveEquippedActor(EquipmentFragment->GetEquipmentType());

	// 清理 fragment 内部引用
	EquipmentFragment->SetEquippedActor(nullptr);
}

void UInvEquipmentComponent::RemoveEquippedActor(const FGameplayTag& EquipmentTypeTag)
{
	if (AInvEquipActor* EquippedActor = FindEquippedActor(EquipmentTypeTag); IsValid(EquippedActor))
	{
		EquippedActors.Remove(EquippedActor);

		// 保证不再附加
		EquippedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// 不需要 RemoveFromRoot，除非你手动 AddToRoot 过
		// EquippedActor->RemoveFromRoot();

		if (!EquippedActor->Destroy())
		{
			// UE_LOG(LogTemp, Warning, TEXT("Destroy failed for %s (IsPendingKill=%d)"),
			// 	*EquippedActor->GetName(),
			// 	EquippedActor->IsPendingKillPending());
		}
	}
}