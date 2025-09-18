// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/Spatial/InvSpatialInventory.cpp

#include "Widgets/Inventory/Spatial/InvSpatialInventory.h"

#include "Inventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/WidgetSwitcher.h"
#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "InventoryManagement/Utils/InvInventoryStatics.h"
#include "Items/Components/InvItemComponent.h"
#include "Widgets/Inventory/GridSlots/InvEquippedGridSlot.h"
#include "Widgets/Inventory/HoverItem/InvHoverItem.h"
#include "Widgets/Inventory/SlottedItems/InvEquippedSlottedItem.h"
#include "Widgets/Inventory/Spatial/InvInventoryGrid.h"
#include "Widgets/ItemDescription/InvItemDescription.h"

void UInvSpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 动态委托通过名字查找并调用回调，依赖反射，因此要加UFUNCTION
	Button_Equippables->OnClicked.AddDynamic(this, &ThisClass::ShowEquippables);
	Button_Consumables->OnClicked.AddDynamic(this, &ThisClass::ShowConsumables);
	Button_Craftables->OnClicked.AddDynamic(this, &ThisClass::ShowCraftables);

	Grid_Equippables->SetOwningCanvas(CanvasPanel);
	Grid_Consumables->SetOwningCanvas(CanvasPanel);
	Grid_Craftables->SetOwningCanvas(CanvasPanel);

	ShowEquippables();

	// 遍历子控件，初始化EquippedGridSlots并对子空间中的委托字段进行回调绑定
	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UInvEquippedGridSlot* EquippedGridSlot = Cast<UInvEquippedGridSlot>(Widget);
		if (IsValid(EquippedGridSlot))
		{
			EquippedGridSlots.Add(EquippedGridSlot);
			EquippedGridSlot->EquippedGridSlotClicked.AddDynamic(this, &ThisClass::EquippedGridSlotClicked);
		}
	});
}

void UInvSpatialInventory::EquippedGridSlotClicked(UInvEquippedGridSlot* EquippedGridSlot, const FGameplayTag& EquipmentTypeTag)
{
	// 检查是否可以装备当前悬停的物品
	if (!CanEquipHoverItem(EquippedGridSlot, EquipmentTypeTag))
		return;
	
	// 创建一个已装备的槽位物品，并将其添加到已装备的网格槽中(调用 EquippedGridSlot->OnItemEquipped())
	UInvHoverItem* HoverItem = GetHoverItem();
	const float TileSize = UInvInventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize();
	UInvEquippedSlottedItem* EquippedSlottedItem = EquippedGridSlot->OnItemEquipped(
		HoverItem->GetInventoryItem(),
		EquipmentTypeTag,
		TileSize
	);
	EquippedSlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked); // 绑定卸下装备的回调
	
	// 通知服务器我们已装备了一个物品（也可能同时取消装备另一个物品）
	UInvInventoryComponent* InventoryComponent = UInvInventoryStatics::GetInvInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));
	
	InventoryComponent->Server_EquipSlotClicked(HoverItem->GetInventoryItem(),nullptr);
	
	// if (GetOwningPlayer()->GetNetMode() != NM_DedicatedServer) // 非客户端则手动调用
	// {
	// 	InventoryComponent->OnItemEquipped.Broadcast(HoverItem->GetInventoryItem());
	// }

	// 清除悬停物品
	Grid_Equippables->ClearHoverItem();
}

void UInvSpatialInventory::EquippedSlottedItemClicked(UInvEquippedSlottedItem* EquippedSlottedItem)
{
	// 移除持有HoverItem时的物品描述
	UInvInventoryStatics::ItemUnhovered(GetOwningPlayer());

	if (IsValid(GetHoverItem()) && GetHoverItem()->IsStackable())
		return;
	
	// **获取要装备的物品**
	UInvInventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr;
	
	// **获取要卸下的物品**
	UInvInventoryItem* ItemToUnequip = EquippedSlottedItem->GetInventoryItem();
	
	// 获取持有要卸下的物品的 **已装备网格槽位**
	UInvEquippedGridSlot* EquippedGridSlot = FindSlotWithEquippedItem(ItemToUnequip);
	
	// 清除此物品所在的已装备网格槽位（将其库存物品设置为 nullptr）
	ClearSlotOfItem(EquippedGridSlot);
	
	// 将先前装备的物品分配为 **悬停物品**
	Grid_Equippables->AssignHoverItem(ItemToUnequip);

	// 从已装备网格槽位中移除 **已装备的槽位物品**
	RemoveEquippedSlottedItem(EquippedSlottedItem);
	
	// 创建一个新的 **已装备槽位物品**（用于我们保存在 HoverItem 中的物品）
	MakeEquippedSlottedItem(EquippedSlottedItem, EquippedGridSlot, ItemToEquip);
	
	UInvInventoryComponent* InventoryComponent = UInvInventoryStatics::GetInvInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));

	// RPC Server_EquipSlotClicked 让所有客户端的对应角色穿戴或卸下对应的装备
	InventoryComponent->Server_EquipSlotClicked(ItemToEquip, ItemToUnequip);
}

void UInvSpatialInventory::MakeEquippedSlottedItem(UInvEquippedSlottedItem* EquippedSlottedItem,
	UInvEquippedGridSlot* EquippedGridSlot, UInvInventoryItem* ItemToEquip)
{
	if (!IsValid(EquippedGridSlot))
		return;
	
	UInvEquippedSlottedItem* SlottedItem = EquippedGridSlot->OnItemEquipped(
		ItemToEquip,
		EquippedSlottedItem->GetEquipmentTypeTag(),
		UInvInventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize()
	);

	if (IsValid(SlottedItem))
		SlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);

	EquippedGridSlot->SetEquippedSlottedItem(SlottedItem);
}

FInvSlotAvailabilityResult UInvSpatialInventory::HasRoomForItem(UInvItemComponent* ItemComponent) const
{
	switch (UInvInventoryStatics::GetItemCategoryFromItemComp(ItemComponent))
	{
	case EInvItemCategory::Equippable:
		return Grid_Equippables->HasRoomForItem(ItemComponent);
	case EInvItemCategory::Consumable:
		return Grid_Consumables->HasRoomForItem(ItemComponent);
	case EInvItemCategory::Craftable:
		return Grid_Craftables->HasRoomForItem(ItemComponent);
	default:
		UE_LOG(LogInventory, Error, TEXT("ItemComponent 没有有效的 Item Category"))
		return FInvSlotAvailabilityResult();
	}
}

FReply UInvSpatialInventory::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	ActiveGrid->DropItem();
	
	return FReply::Handled(); // 表示当前已处理，不希望父级框架或控件响应，不向父类冒泡，冒泡？
}

void UInvSpatialInventory::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(ItemDescription))
		return;

	SetItemDescriptionSizeAndPosition(ItemDescription, CanvasPanel);
}

void UInvSpatialInventory::SetItemDescriptionSizeAndPosition(UInvItemDescription* Description,
                                                             UCanvasPanel* Canvas) const
{
	UCanvasPanelSlot* ItemDescriptionCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(Description);
	if (!IsValid(ItemDescriptionCPS))
		return;

	const FVector2D ItemDescriptionSize = Description->GetBoxSize();
	ItemDescriptionCPS->SetSize(ItemDescriptionSize);

	FVector2D ClampedPosition = UInvWidgetUtils::GetClampedWidgetPosition(
		UInvWidgetUtils::GetWidgetSize(Canvas),
		ItemDescriptionSize,
		UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()));

	ItemDescriptionCPS->SetPosition(ClampedPosition); // 设置位置
}

bool UInvSpatialInventory::CanEquipHoverItem(UInvEquippedGridSlot* EquippedGridSlot,
	const FGameplayTag& EquipmentTypeTag) const
{
	if (!IsValid(EquippedGridSlot) || EquippedGridSlot->GetInventoryItem().IsValid())
		return false;

	UInvHoverItem* HoverItem = GetHoverItem();
	if (!IsValid(HoverItem))
		return false;
 
	UInvInventoryItem* HeldItem = HoverItem->GetInventoryItem();

	return HasHoverItem() && IsValid(HeldItem) && !HoverItem->IsStackable() &&
				HeldItem->GetItemManifest().GetItemCategory() == EInvItemCategory::Equippable &&
					HeldItem->GetItemManifest().GetItemType().MatchesTag(EquipmentTypeTag);
}

float UInvSpatialInventory::GetTileSize() const
{
	return Grid_Equippables->GetTileSize();
}

UInvEquippedGridSlot* UInvSpatialInventory::FindSlotWithEquippedItem(UInvInventoryItem* EquippedItem) const
{
	// 遍历EquippedGridSlots
	auto* FoundEquippedGridSlot = EquippedGridSlots.FindByPredicate(
		[EquippedItem](const UInvEquippedGridSlot* GridSlot)
	{
		return GridSlot->GetInventoryItem() == EquippedItem;	
	});
	
	return FoundEquippedGridSlot ? *FoundEquippedGridSlot : nullptr;
}

void UInvSpatialInventory::ClearSlotOfItem(UInvEquippedGridSlot* EquippedGridSlot)
{
	if (IsValid(EquippedGridSlot))
	{
		EquippedGridSlot->SetInventoryItem(nullptr);
		EquippedGridSlot->SetEquippedSlottedItem(nullptr);
	}
}

void UInvSpatialInventory::RemoveEquippedSlottedItem(UInvEquippedSlottedItem* EquippedSlottedItem)
{
	if (!IsValid(EquippedSlottedItem))
		return;
	
	// （从 **装备槽位物品点击事件** 上解绑）
	if (EquippedSlottedItem->OnEquippedSlottedItemClicked.IsAlreadyBound(this, &ThisClass::EquippedSlottedItemClicked)) // 检查是否绑定特定
	{
		EquippedSlottedItem->OnEquippedSlottedItemClicked.RemoveDynamic(this, &ThisClass::EquippedSlottedItemClicked); // 解绑
	}
	
	// 从父级移除 *已装备的槽位物品 Item*
	EquippedSlottedItem->RemoveFromParent();
}

void UInvSpatialInventory::BroadcastSlotClickedDelegates(UInvInventoryItem* ItemToEquip, UInvInventoryItem* ItemToUnequip) const
{
	UE_LOG(LogTemp, Warning, TEXT("BroadcastSlotClickedDelegates"));
	
	UInvInventoryComponent* InventoryComponent = UInvInventoryStatics::GetInvInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));

	if (GetOwningPlayer()->GetNetMode() != NM_DedicatedServer) // 只要当前不是专用服务器(只跑逻辑的服务器)，就会广播
	{
		InventoryComponent->OnItemEquipped.Broadcast(ItemToEquip);
		InventoryComponent->OnItemUnequipped.Broadcast(ItemToUnequip);
	}
}

void UInvSpatialInventory::OnItemHovered(UInvInventoryItem* Item)
{
	UInvItemDescription* DescriptionWidget = GetItemDescription();
	DescriptionWidget->SetVisibility(ESlateVisibility::Collapsed); // 先折叠，后续会延迟展示

	const auto& Manifest = Item->GetItemManifest();
	
	FTimerDelegate DescriptionTimerDelegate;
	DescriptionTimerDelegate.BindLambda([this, &Manifest, DescriptionWidget]()
	{
		Manifest.AssimilateInventoryFragments(DescriptionWidget); // 将该物品定义的信息展示到ItemDescription上
		GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible); // 取消鼠标对ItemDescription的检测
	});

	// 物品描述延迟DescriptionTimerDelay秒后显示
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);
	GetOwningPlayer()->GetWorldTimerManager().SetTimer(DescriptionTimer, DescriptionTimerDelegate, DescriptionTimerDelay, false);
}

void UInvSpatialInventory::OnItemUnhovered()
{
	GetItemDescription()->SetVisibility(ESlateVisibility::Collapsed);
	
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);
}

bool UInvSpatialInventory::HasHoverItem() const
{
	if (Grid_Equippables->HasHoverItem()) return true;
	if (Grid_Consumables->HasHoverItem()) return true;
	if (Grid_Craftables->HasHoverItem()) return true;

	return false;
}

UInvHoverItem* UInvSpatialInventory::GetHoverItem() const
{
	if (!ActiveGrid.IsValid())
		return nullptr;

	return ActiveGrid->GetHoverItem();
}

void UInvSpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables, Button_Equippables);
}

void UInvSpatialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables, Button_Consumables);
}

void UInvSpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables, Button_Craftables);
}

void UInvSpatialInventory::DisableButton(UButton* Button)
{
	Button_Equippables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}

UInvItemDescription* UInvSpatialInventory::GetItemDescription()
{
	if (!IsValid(ItemDescription))
	{
		ItemDescription = CreateWidget<UInvItemDescription>(GetOwningPlayer(), ItemDescriptionClass);
		CanvasPanel->AddChild(ItemDescription);
	}
	
	return ItemDescription;
}

void UInvSpatialInventory::SetActiveGrid(UInvInventoryGrid* Grid, UButton* Button)
{
	if (ActiveGrid.IsValid())
		ActiveGrid->HideCursor();
	
	ActiveGrid = Grid;
	
	if (ActiveGrid.IsValid())
		ActiveGrid->ShowCursor();
	
	DisableButton(Button); // 禁用特定按钮并启用其他按钮

	Switcher->SetActiveWidget(Grid); // 激活特定控件
}
