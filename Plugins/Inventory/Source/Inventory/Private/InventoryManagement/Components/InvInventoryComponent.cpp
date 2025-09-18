// Fill out your copyright notice in the Description page of Project Settings.
// InventoryManagement/Components/InvInventoryComponent.cpp

#include "InventoryManagement/Components/InvInventoryComponent.h"

#include "BaseGizmos/GizmoElementShared.h"
#include "GameFramework/PlayerState.h"
#include "InventoryManagement/Utils/InvInventoryStatics.h"
#include "Items/InvInventoryItem.h"
#include "Items/Components/InvItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/InvPlayerController.h"
#include "Widgets/Inventory/InventoryBase/InvInventoryBase.h"


UInvInventoryComponent::UInvInventoryComponent() : InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	bInventoryMenuOpen = false;
}

void UInvInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

void UInvInventoryComponent::ToggleInventoryMenu()
{
	if (bInventoryMenuOpen)
		CloseInventoryMenu();
	else
		OpenInventoryMenu();

	OnInventoryMenuToggled.Broadcast(bInventoryMenuOpen);
}

void UInvInventoryComponent::TryAddItem(UInvItemComponent* ItemComponent)
{
	// 根据InventoryBase派生类检查物品是否能放入背包
	FInvSlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent); 

	UInvInventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComponent->GetItemManifest().GetItemType());
	Result.Item = FoundItem;

	// 无法放入背包
	if (Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}
	
	if (Result.Item.IsValid() && Result.bStackable) // 存在该类物品且可堆叠
	{
		// 堆叠该物品，只会更新堆叠数量
		OnStackChange.Broadcast(Result);
		Server_AddStacksToItem(ItemComponent, Result.TotalRoomToFill, Result.Remainder);
	}
	else if (Result.TotalRoomToFill > 0)
	{
		// 该物品不存在，建一个新的物品并更新所有相关槽位。
		Server_AddNewItem(ItemComponent, Result.bStackable ? Result.TotalRoomToFill : 0);
	}
}

void UInvInventoryComponent::Server_AddNewItem_Implementation(UInvItemComponent* ItemComponent, int32 StackCount)
{
	UInvInventoryItem* NewItem = InventoryList.AddEntry(ItemComponent); // 在FastArray中添加元素，FastArray变更传播
	NewItem->SetTotalStackCount(StackCount);

	if (GetOwner()->GetNetMode() == NM_ListenServer || GetOwner()->GetNetMode() == NM_Standalone)
	{
		OnItemAdded.Broadcast(NewItem); // 如果是专用服务器情况，会在FastArray的客户端回调中广播
	}
	
	ItemComponent->PickedUp();
}

void UInvInventoryComponent::Server_AddStacksToItem_Implementation(UInvItemComponent* ItemComponent, const int32 StackCount, const int32 Remainder)
{
	// 确定服务器中是否存在该Item
	const FGameplayTag& ItemType = IsValid(ItemComponent) ? ItemComponent->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInvInventoryItem* Item = InventoryList.FindFirstItemByType(ItemType);
	if (!IsValid(Item))
		return;
	
	Item->SetTotalStackCount(Item->GetTotalStackCount() + StackCount); // 数量堆叠
	
	if (Remainder == 0) // 拾取的物品的数量能完全填充进仓库中，那么就表现拾取特效并把物品从世界中销毁
		ItemComponent->PickedUp();
	else if (FInvStackableFragment* StackableFragment = ItemComponent->GetItemManifest().GetFragmentOfTypeMutable<FInvStackableFragment>())
	{
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInvInventoryComponent::Server_DropItem_Implementation(UInvInventoryItem* Item, int32 StackCount)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - StackCount;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	SpawnDroppedItem(Item, StackCount);
}

void UInvInventoryComponent::SpawnDroppedItem(UInvInventoryItem* Item, int32 StackCount)
{
	const APawn* OwningPawn = OwningController->GetPawn();
	FVector RotatedForward = OwningPawn->GetActorForwardVector(); // 获取角色面朝方向
	RotatedForward = RotatedForward.RotateAngleAxis( // 绕某个轴旋转
		FMath::RandRange(DropSpawnAngleMin, DropSpawnAngleMax), // 旋转随机角度
		FVector::UpVector); // 绕Y轴旋转

	// 计算生成位置和生成角度
	FVector SpawnLocation = OwningPawn->GetActorLocation() // 根部组件(角色胶囊体)中心位置
	+ RotatedForward * FMath::RandRange(DropSpawnDistanceMin, DropSpawnDistanceMax); // 角色前方扇形的随机距离
	SpawnLocation.Z -= RelativeSpawnElevation; // 调整生成位置高度
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	// 生成Item
	FInvItemManifest& ItemManifest = Item->GetItemManifestMutable(); // Mutable
	if (FInvStackableFragment* StackableFragment = ItemManifest.GetFragmentOfTypeMutable<FInvStackableFragment>())
	{
		StackableFragment->SetStackCount(StackCount); // 因为要更新StackCount，所以要GetFragmentOfTypeMutable而不是返回const
	}
	ItemManifest.SpawnPickUpActor(this, SpawnLocation, SpawnRotation);
}

void UInvInventoryComponent::Server_ConsumeItem_Implementation(UInvInventoryItem* Item)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - 1;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	if (FInvConsumableFragment* ConsumableFragment = Item->GetItemManifestMutable().GetFragmentOfTypeMutable<FInvConsumableFragment>())
	{
		ConsumableFragment->OnConsume(OwningController.Get());
	}
}

void UInvInventoryComponent::Server_EquipSlotClicked_Implementation(UInvInventoryItem* ItemToEquip, UInvInventoryItem* ItemToUnequip)
{
	// 基本安全校验
	if (!IsValid(ItemToEquip) && !IsValid(ItemToUnequip))
	{
		return;
	}

	// 确保在服务端/authority 上执行
	AActor* CompOwner = GetOwner();
	if (!IsValid(CompOwner) || !CompOwner->HasAuthority())
	{
		return;
	}

	// 是否在服务器上创建 authoritative 装备（replicated actor）
	const bool bServerShouldEquip = true; // 根据你游戏逻辑调整：true = server spawn authoritative equip; false = server only notify client for preview

	if (bServerShouldEquip)
	{
		// 服务器权威：广播给监听（比如角色的 EquipmentComponent 会响应并 spawn replicated actor）
		OnItemEquipped.Broadcast(ItemToEquip);
		OnItemUnequipped.Broadcast(ItemToUnequip);
	}

	// 无论 server 是否 spawn，都应明确通知目标玩家的 InventoryComponent 去执行 owner-view（client-only）
	if (OwningController.IsValid())
	{
		UInvInventoryComponent* TargetInvComp = UInvInventoryStatics::GetInvInventoryComponent(OwningController.Get());
		if (IsValid(TargetInvComp))
		{
			// 在目标组件上调用 Client RPC（引擎会把这个 RPC 发到 OwningController 所属客户端）
			TargetInvComp->Client_OnOwnerViewEquipped(ItemToEquip, ItemToUnequip);
		}
	}
}



// void UInvInventoryComponent::Multicast_EquipSlotClicked_Implementation(UInvInventoryItem* ItemToEquip,
// 	UInvInventoryItem* ItemToUnequip)
// {
// 	// Equipment Component 监听响应
//
// 	if (!OwningController->HasAuthority())
// 		return;
// 	
// 	OnItemEquipped.Broadcast(ItemToEquip);
// 	OnItemUnequipped.Broadcast(ItemToUnequip);
// }

void UInvInventoryComponent::Client_OnOwnerViewEquipped_Implementation(UInvInventoryItem* ItemToEquip, UInvInventoryItem* ItemToUnequip)
{
	// 如果组件的 owner 不是本地 controller，那么跳过（防御式）
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (!PC->IsLocalController())
		{
			return;
		}
	}

	// 广播 OwnerView 委托，让本地的 EquipmentComponent（proxy）去做本地预览/挂载
	OnItemEquipped_OwnerView.Broadcast(ItemToEquip);
	OnItemUnequipped_OwnerView.Broadcast(ItemToUnequip);
}

// void UInvInventoryComponent::InitiateEquipPrediction(UInvInventoryItem* ItemToEquip, const FGameplayTag& SlotTag)
// {
// 	if (!IsValid(ItemToEquip))
// 		return;
// 	
// 	uint32 Seq = NextClientSequence++;
// 	FPendingEquipEntry Entry;
// 	Entry.ClientSequence = Seq;
// 	Entry.SlotTag = SlotTag;
// 	Entry.Item = ItemToEquip;
// 	Entry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
// 	
// 	Entry.PredictedProxyActor = SpawnPredictedEquipActorOnProxy(ItemToEquip, Seq, SlotTag);
// 	Entry.PredictedCharacterActor = SpawnPredictedEquipActorOnCharacter(ItemToEquip, Seq, SlotTag);
// 	
// 	PendingEquips.Add(Seq, Entry);
// 	
// 	// ProxyMesh穿戴装备
// 	OnItemEquipped_OwnerView.Broadcast(ItemToEquip);
// 	
// 	
// 	// 
// 	Server_RequestEquipAction(ItemToEquip, Seq, SlotTag);
// }
//
// void UInvInventoryComponent::InitiateUnequipPrediction(UInvInventoryItem* ItemToUnequip, const FGameplayTag& SlotTag)
// {
// 	
// }
//
// void UInvInventoryComponent::Server_RequestEquipAction_Implementation(UInvInventoryItem* ItemToEquip, uint32 ClientSequence,
//                                                                 const FGameplayTag& SlotTag)
// {	
// 	
// }	
// 	
// void UInvInventoryComponent::Client_AckEquipAction_Implementation(uint32 ClientSequence, UInvInventoryItem* Item,
// 	bool bSuccess, const FGameplayTag& SlotTag)
// {	
// 	
// }

void UInvInventoryComponent::AddRepSubObj(UObject* SubObj)
{
	/**
	 * IsUsingRegisteredSubObjectList(): 判断是否使用注册表的方式来进行子对象复制(即把一个 UObject及其属性状态，从服务器同步到客户端)
	 * IsReadyForReplication(): 组件是否已经“准备好”进入网络复制阶段，即当组件所属的 Actor 完成初始化（InitializeComponent）
	 *		并且即将或已经进入网络复制生命周期时，底层会调用 ReadyForReplication()，此后 IsReadyForReplication() 返回 true
	 */
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		// 即告诉引擎，把这个 SubObj 当作我的子对象，在以后每次网络同步（Replication Tick）时，也要一并序列化并发给客户端
		AddReplicatedSubObject(SubObj); 
	}
}

void UInvInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ConstructInventory();
		
}

void UInvInventoryComponent::ConstructInventory()
{
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(), TEXT("Inventory Component 应该被 Player Controller 拥有"));

	if (!OwningController->IsLocalController()) // 本地控制器才创建仓库
		return;

	/*
	 * 创建仓库Menu
	 * 第一个参数的类型"OwnerType"是UObject，所有要Get获取指针
	 * OwningObject管理控件的生命周期
	 * 第二个参数是要实例化的控件类
	 * 第一个模板参数必须显式指定，它也是CreateWidget()的返回类型；
	 * 第二个模板参数可以从第一个函数参数中推导而出，因此可以不显式指定
	 */ 
	InventoryMenu = CreateWidget<UInvInventoryBase>(OwningController.Get(), InventoryMenuClass);

	InventoryMenu->AddToViewport(); // 将InventoryMenu添加到ViewPort的层级中，与CloseInventoryMenu不冲突(存在不一定显示)

	CloseInventoryMenu();
}

void UInvInventoryComponent::OpenInventoryMenu()
{
	if (!IsValid(InventoryMenu))
		return;

	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen = true;

	if (!OwningController.IsValid())
		return;

	FInputModeGameAndUI InputMode; // 构造输入模式，该输入模式表示同时处理游戏世界和UI界面的输入
	OwningController->SetInputMode(InputMode); // 设置构造好的输入模式
	OwningController->SetShowMouseCursor(true); 
}

void UInvInventoryComponent::CloseInventoryMenu()
{
	if (!IsValid(InventoryMenu))
		return;

	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen = false;

	if (!OwningController.IsValid())
		return;

	FInputModeGameOnly InputMode; // 只处理游戏世界的输入
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}

// AInvEquipActor* UInvInventoryComponent::SpawnPredictedEquipActorOnProxy(UInvInventoryItem* Item, uint32 ClientSequence,
// 	const FGameplayTag& SlotTag)
// {
// }
//
// AInvEquipActor* UInvInventoryComponent::SpawnPredictedEquipActorOnCharacter(UInvInventoryItem* Item,
// 	uint32 ClientSequence, const FGameplayTag& SlotTag)
// {
// }
//
// AInvEquipActor* UInvInventoryComponent::SpawnPredictedEquipActorForPlayer(APawn* PlayerPawn, UInvInventoryItem* Item,
// 	const FGameplayTag& SlotTag)
// {
// }
//
// bool UInvInventoryComponent::ValidateItemBelongsToController(UInvInventoryItem* Item, APlayerController* PC) const
// {
// }
//
// bool UInvInventoryComponent::ValidateEquipConditions(UInvInventoryItem* Item, APlayerController* PC,
// 	const FGameplayTag& SlotTag) const
// {
// }
//
// void UInvInventoryComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
//                                            FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
// }
//
// void UInvInventoryComponent::RollbackLocalPrediction(uint32 ClientSequence)
// {
// }
//
// void UInvInventoryComponent::RequestServerInventorySync()
// {
// }
//
//
// void UInvInventoryComponent::DestroyPredictedActorsBySequence(uint32 ClientSequence)
// {
// }
//
// void UInvInventoryComponent::RollbackLocalEquip(uint32 ClientSequence)
// {
// }




