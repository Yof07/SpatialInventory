// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Inventory/Spatial/InvInventoryGrid.cpp

#include "Widgets/Inventory/Spatial/InvInventoryGrid.h"

#include "IDetailTreeNode.h"
#include "Inventory.h"
#include "WeightMapTypes.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "InventoryManagement/Utils/InvInventoryStatics.h"
#include "Items/Fragments/InvFragmentTags.h"
#include "Widgets/Inventory/GridSlots/InvGridSlot.h"
#include "Widgets/Inventory/SlottedItems/InvSlottedItem.h"
#include "Widgets/Utils/InvWidgetUtils.h"
#include "Widgets/Inventory/HoverItem/InvHoverItem.h"
#include "Widgets/ItemPopUp/InvItemPopUp.h"

void UInvInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	InventoryComponent = UInvInventoryStatics::GetInvInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this, &ThisClass::AddItem);

	InventoryComponent->OnStackChange.AddDynamic(this, &ThisClass::AddStacks);
}

void UInvInventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	const FVector2D CanvasPosition = UInvWidgetUtils::GetWidgetPosition(CanvasPanel); 
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

	// CursorExitedCanvas 更新 bLastMouseWithinCanvas 和 bMouseWithinCanvas
	if (CursorExitedCanvas(CanvasPosition, UInvWidgetUtils::GetWidgetSize(CanvasPanel),MousePosition)) // 鼠标离开画布
	{
		return;
	}

	// 包含了ItemDropIndex的更新
	UpdateTileParameters(CanvasPosition, MousePosition);
}
	
void UInvInventoryGrid::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{	
	if (!bMouseWithinCanvas)
		return;
	
	// 计算tile的象限、索引和坐标
	const FIntPoint HoveredTileCoordinates = CalculateHoveredCoordinates(CanvasPosition, MousePosition);
	
	LastTileParameters = TileParameters;
	TileParameters.TileCoordinates = HoveredTileCoordinates;
	TileParameters.TileIndex = UInvWidgetUtils::GetIndexFromPosition(HoveredTileCoordinates, Columns);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition, MousePosition);
	
	// 处理 grid slot 的高亮与否、其他字段的更新和初始化(如ItemDropIndex)，HoverItem存在时才调用
	OnTileParametersUpdated(TileParameters);
}
	
void UInvInventoryGrid::OnTileParametersUpdated(const FInvTileParameters& Parameters)
{	
	if (!IsValid(HoverItem))
		return;
	
	// 获取hover item的长宽(dimension)，才知道要高亮的部分有多大
	const FIntPoint Dimensions = HoverItem->GetGridDimensions();
	
	// 计算高亮的起始坐标
	const FIntPoint StartingCoordinate = CalculateStartingCoordinate(Parameters.TileCoordinates, Dimensions, Parameters.TileQuadrant);
	ItemDropIndex = UInvWidgetUtils::GetIndexFromPosition(StartingCoordinate, Columns);
	
	// 检查悬停位置(以HoverItem对GridSlots悬停的位置进行检查)
	CurrentQueryResult = CheckHoverPosition(StartingCoordinate, Dimensions); 
	
	// 悬停的位置为空，没有放下物品
	if (CurrentQueryResult.bHasSpace) 
	{
		HighlightSlots(ItemDropIndex, Dimensions); 
		return;
	}
	UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
	
	// 悬停的位置非空，有放下物品
	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		const FInvGridFragment* GridFragment = GetFragment<FInvGridFragment>(CurrentQueryResult.ValidItem.Get(), FragmentTags::GridFragment);
		if (!GridFragment)
			return;
	
		// 交换或堆叠时变灰
		ChangeHoverType(CurrentQueryResult.UpperLeftIndex, GridFragment->GetGridSize(), EInvGridSlotState::GrayedOut);
	}
}

FInvSpaceQueryResult UInvInventoryGrid::CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions)
{
	FInvSpaceQueryResult Result;
	
	// dimension(hover item的长宽) 是否在网格内
	if (!IsInGridBounds(UInvWidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions))
		return Result; // 默认没有空间
	
	Result.bHasSpace = true;
	
	// 如果多个索引都被同一物体占据，那要判断这些索引是否有相同的左上角索引，即判断那个矩形(Dimensions)所占据的地方是否有>=2个物体
	TSet<int32> OccupiedUpperLeftIndices;
	UInvInventoryStatics::ForEach2D(GridSlots, UInvWidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions, Columns,
		[&](const UInvGridSlot* GridSlot)
		{
			if (GridSlot->GetInventoryItem().IsValid())
			{
				OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
				Result.bHasSpace = false;
			}
		});
	
	// 如果有物体，是否只有一个？有一个则表示可以进行交换
	if (OccupiedUpperLeftIndices.Num() == 1)
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();
		Result.ValidItem = GridSlots[Index]->GetInventoryItem();
		Result.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	}
	
	return Result;
}

bool UInvInventoryGrid::CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundarySize,
	const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	bMouseWithinCanvas = UInvWidgetUtils::IsWithinBounds(BoundaryPos, BoundarySize, Location);

	if (!bMouseWithinCanvas && bLastMouseWithinCanvas) // 鼠标离开了canvas
	{
		UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
		return true;
	}
	
	return false;
}

void UInvInventoryGrid::HighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas)
		return;

	UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);

	UInvInventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInvGridSlot* GridSlot)
	{
		GridSlot->SetOccupiedTexture();
	});

	LastHighlightedDimensions = Dimensions;
	LastHighlightedIndex = Index;
}

void UInvInventoryGrid::UnHighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UInvInventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInvGridSlot* GridSlot)
	{
		if (GridSlot->IsAvailable())
			GridSlot->SetUnOccupiedTexture();
		else
			GridSlot->SetOccupiedTexture();
	});
}

void UInvInventoryGrid::ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EInvGridSlotState GridSlotState)
{
	UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
	UInvInventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns,
		[State = GridSlotState](UInvGridSlot* GridSlot)
	{
		switch (State)
		{
		case EInvGridSlotState::Occupied:
			GridSlot->SetOccupiedTexture();
			break;
		case EInvGridSlotState::Unoccupied:
			GridSlot->SetUnOccupiedTexture();
			break;
		case EInvGridSlotState::GrayedOut:
			GridSlot->SetGrayedOutTexture();
			break;
		case EInvGridSlotState::Selected:
			GridSlot->SetSelectedTexture();
			break;
		}
	});

	LastHighlightedIndex = Index;
	LastHighlightedDimensions = Dimensions;
}

FIntPoint UInvInventoryGrid::CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions,
                                                         const EInvTileQuadrant Quadrant) const
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;

	FIntPoint StartingCoord;

	switch (Quadrant)
	{
	case EInvTileQuadrant::TopLeft: 
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5 * Dimensions.X);
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5 * Dimensions.Y);
		break;
	case EInvTileQuadrant::TopRight:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5 * Dimensions.X) + HasEvenWidth;
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5 * Dimensions.Y);
		break;
	case EInvTileQuadrant::BottomLeft:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5 * Dimensions.X);
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5 * Dimensions.Y) + HasEvenHeight;
		break;
	case EInvTileQuadrant::BottomRight:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5 * Dimensions.X) + HasEvenWidth;
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5 * Dimensions.Y) + HasEvenHeight;
		break;
	default:
		UE_LOG(LogInventory, Error, TEXT("无效象限"));
		return FIntPoint(-1,-1);
	}

	return StartingCoord;
}

FIntPoint UInvInventoryGrid::CalculateHoveredCoordinates(const FVector2D& CanvasPosition,
                                                         const FVector2D& MousePosition) const
{
	return FIntPoint
	{
		static_cast<int32>(FMath::FloorToInt(MousePosition.X - CanvasPosition.X) / TileSize),
		static_cast<int32>(FMath::FloorToInt(MousePosition.Y - CanvasPosition.Y) / TileSize)
	};
}

EInvTileQuadrant UInvInventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const
{
	// 计算鼠标位置在一个网格单元(GridSLot)内的相对位置来确定鼠标处于哪个象限
	const float TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X, TileSize);
	const float TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y, TileSize);

	// 确定鼠标处于哪个象限
	const bool bIsTop = TileLocalY < TileSize / 2.f;
	const bool bIsLeft = TileLocalX < TileSize / 2.f;

	EInvTileQuadrant HoveredTileQuadrant = EInvTileQuadrant::None;
	if (bIsTop && bIsLeft)
		HoveredTileQuadrant = EInvTileQuadrant::TopLeft;
	else if (bIsTop && !bIsLeft)
		HoveredTileQuadrant = EInvTileQuadrant::TopRight;
	else if (!bIsTop && bIsLeft)
		HoveredTileQuadrant = EInvTileQuadrant::BottomLeft;
	else if (!bIsTop && !bIsLeft)
		HoveredTileQuadrant = EInvTileQuadrant::BottomRight;

	return HoveredTileQuadrant;
}

FInvSlotAvailabilityResult UInvInventoryGrid::HasRoomForItem(const UInvItemComponent* ItemComponent) // ItemComponent
{
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInvSlotAvailabilityResult UInvInventoryGrid::HasRoomForItem(const UInvInventoryItem* Item) // Item
{
	return HasRoomForItem(Item->GetItemManifest());
}

FInvSlotAvailabilityResult UInvInventoryGrid::HasRoomForItem(const FInvItemManifest& Manifest) // Manifest
{
	FInvSlotAvailabilityResult Result;
	
	// 获取堆叠片段来确定物品是否可堆叠，可堆叠返回有效指针，不可堆叠返回空指针
	const FInvStackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInvStackableFragment>();
	Result.bStackable = StackableFragment != nullptr;
	
	// 确定要添加多少数量。
	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = StackableFragment ? StackableFragment->GetStackCount() : 1; // 堆叠碎片中的数量
	
	TSet<int32> CheckedIndices; // 维护一个已检查索引集合来查重
	// 遍历每个格子，根据每个格子的信息构造对应的FInvSlotAvailability，并更新FInvSlotAvailabilityResult的其他字段
	for (const auto& GridSlot : GridSlots)
	{	
		// 如果没有更多需要填充的，提前退出循环。
		if (AmountToFill == 0)
			break;
		
		// 该索引是否已被占用？已检索则跳过
		if (IsIndexClaimed(CheckedIndices, GridSlot->GetIndex()))
			continue;
		
		// 物体必须在网格边界内
		if (!IsInGridBounds(GridSlot->GetIndex(), GetItemDimensions(Manifest)))
			continue;
		
		// 该物品能放在这里吗？（即是否超出网格边界？）
		TSet<int32> TentativelyClaimed;
		if (!HasRoomAtIndex(GridSlot, GetItemDimensions(Manifest), CheckedIndices, TentativelyClaimed, Manifest.GetItemType(), MaxStackSize))
			continue;
		
		// 要填充多少？
	    const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill, GridSlot);
		if (AmountToFillInSlot == 0)
			continue;
	
		CheckedIndices.Append(TentativelyClaimed);
		
		// 更新剩余待填充的数量
		Result.TotalRoomToFill += AmountToFillInSlot;
		Result.SlotAvailabilities.Emplace(
			FInvSlotAvailability
			{ 
				HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetIndex(),
				Result.bStackable ? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
		);
		
		AmountToFill -= AmountToFillInSlot;
		
		// 剩余的数量是多少？
		Result.Remainder = AmountToFill;
		
		if (AmountToFill == 0)
			return Result;
	}	
	
	return Result; 
}		

bool UInvInventoryGrid::HasRoomAtIndex(const UInvGridSlot* GridSlot, const FIntPoint& Dimensions,
                                       const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
                                       const FGameplayTag& ItemType, const int32 MaxStackSize)
{
	// 该索引处还有空间吗？（即是否有其他物品阻挡？）
	bool bHasRoomAtIndex = true;

	UInvInventoryStatics::ForEach2D(GridSlots, GridSlot->GetIndex(), Dimensions, Columns,
	[&](const UInvGridSlot* SubGridSlot) // SubGridSlot就是以GridSlot->GetIndex()为左上角的Dimensions范围的矩形内的GridSlot
	{
		// 对于Dimensions范围内的GridSlot，都去检查槽约束
		if (CheckSlotConstraints(GridSlot, SubGridSlot, CheckedIndices, OutTentativelyClaimed, ItemType, MaxStackSize))
			OutTentativelyClaimed.Add(SubGridSlot->GetIndex());
		else
			bHasRoomAtIndex = false;
	});

	return bHasRoomAtIndex;
}

bool UInvInventoryGrid::CheckSlotConstraints(const UInvGridSlot* GridSlot, const UInvGridSlot* SubGridSlot,
                                             const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
                                             const FGameplayTag& ItemType, const int32 MaxStackSize) const
{
	// 该SubGridSlot的Index之前是否被检查过
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetIndex()))
		return false;
	
	// 这个子网格槽位有物品吗
	if (!HasValidItem(SubGridSlot))
	{
		OutTentativelyClaimed.Add(SubGridSlot->GetIndex()); 
		return true;
	}
	
	// 该网格槽是左上角的槽吗
	if (!IsUpperLeftSlot(GridSlot, SubGridSlot))
		return false;
	
	// SubGridSlot需要可堆叠
	const UInvInventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
	if (!SubItem->IsStackable())
		return false;
	
	// 这个物品的类型和我要添加的物品的类型相同
	if (!DoesItemTypeMatch(SubItem, ItemType))
		return false;
	
	// 堆叠未满
	if (GridSlot->GetStackCount() != MaxStackSize)
		return false;
	
	return true;
}

bool UInvInventoryGrid::HasValidItem(const UInvGridSlot* GridSlot) const
{
	return GridSlot->GetInventoryItem().IsValid();
}

bool UInvInventoryGrid::IsUpperLeftSlot(const UInvGridSlot* GridSlot, const UInvGridSlot* SubGridSlot) const
{
	return GridSlot->GetUpperLeftIndex() == SubGridSlot->GetIndex();
}

bool UInvInventoryGrid::DoesItemTypeMatch(const UInvInventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

bool UInvInventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num())
		return false;

	// 得到物体的矩形的最右下角的行列数
	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X; 
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;

	return EndColumn <= Columns && EndRow <= Rows;
}

int32 UInvInventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize,
	const int32 AmountToFill, const UInvGridSlot* GridSlot) const
{
	// 计算剩余槽内容量
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot); // 注意，这里使用的是InvInventoryGrid类中的GetStackAmount
	// 如果可堆叠，则获取剩余槽内容量和要叠加数量的最小值，否则返回1
	return bStackable ? FMath::Min(RoomInSlot, AmountToFill) : 1;
}

int32 UInvInventoryGrid::GetStackAmount(const UInvGridSlot* GridSlot) const
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount();
	// 如果当前槽不含有物体数量(说明不是左上角槽)，那么就找出左上角槽
	if (const int UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE) // 判断该槽是否为左上角槽
	{
		UInvGridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentSlotStackCount = UpperLeftGridSlot->GetStackCount();
	}

	return CurrentSlotStackCount;
}

FIntPoint UInvInventoryGrid::GetItemDimensions(const FInvItemManifest& Manifest) const
{
	const FInvGridFragment* GridFragment = Manifest.GetFragmentOfType<FInvGridFragment>();
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
}

void UInvInventoryGrid::AddStacks(const FInvSlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get()))
		return;
	
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		if (Availability.bItemAtIndex) // 有物品在这
		{
			const auto& GridSlot = GridSlots[Availability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availability.Index);
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
			GridSlot->SetStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
		}
		else // 没物品在这
		{
			AddItemAtIndex(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountToFill);
			UpdateGridSlots(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountToFill);
		}
	}
}

void UInvInventoryGrid::OnSlottedItemClicked(const int32 GridIndex, const FPointerEvent& MouseEvent)
{
	UInvInventoryStatics::ItemUnhovered(GetOwningPlayer()); // 点击物品悬浮后，物品描述版消失
	
	check(GridSlots.IsValidIndex(GridIndex));

	UInvInventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();

	// 当前没有悬浮物品左键点击
	if (!IsValid(HoverItem) && IsLeftClick(MouseEvent))
	{
		// 创建 hover item 然后把 slotted item 移除
		PickUp(ClickedInventoryItem, GridIndex);
		return;
	}

	// 右键点击
	if (IsRightClick(MouseEvent))
	{
		CreateItemPopUp(GridIndex); // 创建ItemPopUp，就是物品的操作面板
		return;
	}

	// 有悬浮物品且左键点击，则判断点击位置的物品是否可以堆叠或交换
	// 悬停的物品和点击的库存物品是否属于同一类型，并且可以堆叠？
	if (IsSameStackable(ClickedInventoryItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInvStackableFragment* StackableFragment = ClickedInventoryItem->GetItemManifest().GetFragmentOfType<FInvStackableFragment>();
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize();
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount; // 在堆叠满之前一个物品还可以堆叠的数量
		const int32 HoveredStackCount = HoverItem->GetStackCount(); // 悬浮物品的堆叠数量
		
		// 我们应该交换它们的堆叠数量吗？ ()
		if (ShouldSwapStackCounts(RoomInClickedSlot, HoveredStackCount, MaxStackSize))
		{
			SwapStackCounts(ClickedStackCount, HoveredStackCount, GridIndex); // 交换堆叠数量
			return;
		}
		
		// 我们应该消耗悬停物品的堆叠吗？ (悬停物品的数量 <= 剩余数量)，即悬浮的物品a和网格中的物品a叠加在一起，因为它们的数量加起来不超过最大数量
		if (ShouldConsumeHoverItemStacks(HoveredStackCount, RoomInClickedSlot))
		{
			ConsumeHoverItemStacks(ClickedStackCount, HoveredStackCount, GridIndex);
			return;
		}
		
		// 我们应该将堆叠填充到点击的物品上吗？（悬停物品的数量大于可填充数量，多得填充不完，消耗悬停物品的数量来填充，但是不消除悬停物品）
		if (ShouldFillInStack(RoomInClickedSlot, HoveredStackCount))
		{
			FillInStack(RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot, GridIndex);
			return;
		}
		
		// 点击的格子中没有剩余空间
		if (RoomInClickedSlot == 0)
			return;
	}
	
	// 物品不可堆叠或物品种类不同，直接交换
	if (CurrentQueryResult.ValidItem.IsValid()) 
		SwapWithHoverItem(ClickedInventoryItem, GridIndex);
}

bool UInvInventoryGrid::IsRightClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

bool UInvInventoryGrid::IsLeftClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

void UInvInventoryGrid::PickUp(UInvInventoryItem* ClickedInventoryItemItem, const int32 GridIndex)
{
	// 创建HoverItem
	AssignHoverItem(ClickedInventoryItemItem, GridIndex, GridIndex);

	// 移除网格中的Item
	RemoveItemFromGrid(ClickedInventoryItemItem, GridIndex);
	
}

void UInvInventoryGrid::AssignHoverItem(UInvInventoryItem* InventoryItem)
{
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInvHoverItem>(GetOwningPlayer(), HoverItemClass);
	}
	
	const FInvGridFragment* GridFragment = GetFragment<FInvGridFragment>(InventoryItem, FragmentTags::GridFragment);
	const FInvImageFragment* ImageFragment = GetFragment<FInvImageFragment>(InventoryItem, FragmentTags::IconFragment);

	if (!GridFragment || !ImageFragment)
		return;

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.SetImageSize(DrawSize * UWidgetLayoutLibrary::GetViewportScale(this));
	IconBrush.DrawAs = ESlateBrushDrawType::Image;

	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetIsStackable(InventoryItem->IsStackable());

	// SetMouseCursorWidget：把鼠标换成一个 UMG Widget，及创建的HoverItem跟随鼠标移动
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem); 
}

void UInvInventoryGrid::AssignHoverItem(UInvInventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);

	HoverItem->SetPreviousGridIndex(PreviousGridIndex);
	HoverItem->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
}

void UInvInventoryGrid::RemoveItemFromGrid(UInvInventoryItem* InventoryItem, const int32 GridIndex)
{
	const FInvGridFragment* GridFragment = GetFragment<FInvGridFragment>(InventoryItem, FragmentTags::GridFragment);
	if (!GridFragment)
		return;

	UInvInventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns,
		[&](UInvGridSlot* GridSlot)
		{
			GridSlot->SetInventoryItem(nullptr);
			GridSlot->SetUpperLeftIndex(INDEX_NONE);
			GridSlot->SetUnOccupiedTexture();
			GridSlot->SetAvailable(true);
			GridSlot->SetStackCount(0);
		}
	);

	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInvSlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
}

void UInvInventoryGrid::AddItem(UInvInventoryItem* Item)
{
	if (!MatchesCategory(Item))
		return;

	FInvSlotAvailabilityResult Result = HasRoomForItem(Item);

	AddItemToIndices(Result, Item);
	// 创建一个widget去展示物品的icon，并把它添加到网格中的具体位置
}

void UInvInventoryGrid::AddItemToIndices(const FInvSlotAvailabilityResult& Result, UInvInventoryItem* NewItem)
{	
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
		UpdateGridSlots(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill); // 对被放置了物品的GridSlot进行更新
	}

}

void UInvInventoryGrid::AddItemAtIndex(UInvInventoryItem* Item, const int32 Index, const bool bStackable,
	const int32 StackAmount)
{
	// 获取Item的网格尺寸(GridDimension),GridFragment中存储.
	const FInvGridFragment* GridFragment = GetFragment<FInvGridFragment>(Item, FragmentTags::GridFragment);
	// 获取Item的icon，imageFragment中存储.
	const FInvImageFragment* ImageFragment = GetFragment<FInvImageFragment>(Item, FragmentTags::IconFragment);

	if (!GridFragment || !ImageFragment)
		return;

	// 就是创建仓库物品widget
	UInvSlottedItem* SlottedItem = CreateSlottedItem(Item, bStackable, StackAmount, GridFragment, ImageFragment, Index);

	// 将SlottedItem添加到canvas panel上，让玩家可以点击到SlottedItem
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	
	SlottedItems.Add(Index, SlottedItem);
}

UInvSlottedItem* UInvInventoryGrid::CreateSlottedItem(UInvInventoryItem* Item, const bool bStackable,const int32 StackAmount,
	const FInvGridFragment* GridFragment, const FInvImageFragment* ImageFragment, const int32 Index) const
{
	UInvSlottedItem* SlottedItem = CreateWidget<UInvSlottedItem>(GetOwningPlayer(), SlottedItemClass);
	SlottedItem->SetInventoryItem(Item);
	SetSlottedItemImage(SlottedItem, GridFragment, ImageFragment); // 设置这个控件的展示图像
	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetIsStackable(bStackable);

	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);

	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &ThisClass::OnSlottedItemClicked);
	
	return SlottedItem;
}

void UInvInventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInvGridFragment* GridFragment,
	UInvSlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem); // 获取子控件的UCanvasPanelSlot来调整绘制的大小位置等
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPos = UInvWidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInvInventoryGrid::UpdateGridSlots(UInvInventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));

	if (bStackableItem)
		GridSlots[Index]->SetStackCount(StackAmount);

	const FInvGridFragment* GridFragment = GetFragment<FInvGridFragment>(NewItem, FragmentTags::GridFragment);
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	
	UInvInventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns,
		[&](UInvGridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(NewItem); 
		GridSlot->SetUpperLeftIndex(Index); 
		GridSlot->SetOccupiedTexture();
		GridSlot->SetAvailable(false);
	});
}

bool UInvInventoryGrid::IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const
{
	return CheckedIndices.Contains(Index);
}

FVector2D UInvInventoryGrid::GetDrawSize(const FInvGridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2; // 格子的总长减去左右边距
	FVector2D IconSize = GridFragment->GetGridSize() * IconTileWidth; // 实际要占据的长宽
	return IconSize;
}

void UInvInventoryGrid::SetSlottedItemImage(const UInvSlottedItem* SlottedItem, const FInvGridFragment* GridFragment,
	const FInvImageFragment* ImageFragment) const
{
	// FSlateBrush设置，设置要绘制的图像、长宽等
	FSlateBrush Brush; // 是Slate里用来描述如何绘制图片、九宫格、边框等的核心结构体
	Brush.SetResourceObject(ImageFragment->GetIcon()); // 指定要绘制的图像
	Brush.DrawAs = ESlateBrushDrawType::Image; 
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}

void UInvInventoryGrid::ConstructGrid()
{
	GridSlots.Reserve(Rows * Columns); // 预定大小，提前开辟空间，不用自动调整大小，提升性能

	for (int32 j = 0; j < Rows; j++) // 行
	{
		for (int32 i = 0; i < Columns; i++) // 列
		{
			UInvGridSlot* GridSlot = CreateWidget<UInvGridSlot>(this, GridSlotClass);
			CanvasPanel->AddChild(GridSlot);

			const FIntPoint TilePosition = FIntPoint(i, j);
			GridSlot->SetIndex(UInvWidgetUtils::GetIndexFromPosition(TilePosition, Columns)); // 设置单个网格的索引

			// 根据画板插槽设置网格的大小和位置
			UCanvasPanelSlot* GridCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot); // 获取网格的画板插槽，画板插槽的作用是控制画板子控件的大小、位置等参数
			GridCPS->SetSize(FVector2D(TileSize));
			GridCPS->SetPosition(TilePosition * TileSize);

			GridSlots.Add(GridSlot);
			
			GridSlot->GridSlotClicked.AddDynamic(this, &ThisClass::OnGridSlotClicked);
			GridSlot->GridSLotHovered.AddDynamic(this, &ThisClass::OnGridSlotHovered);
			GridSlot->GridSLotUnHovered.AddDynamic(this, &ThisClass::OnGridSlotUnHovered);
		}
	}
}
	
void UInvInventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (!IsValid(HoverItem)) // 如果没有悬浮物品，就返回
		return;

	if (!GridSlots.IsValidIndex(ItemDropIndex)) // 如果你要放置物品的下标不是有效下标
		return;

	// 点击了一个有物品的位置
	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex)) // UpperLeftIndex 是指在路径上阻挡的物品的左上角坐标对应的索引
	{
		OnSlottedItemClicked(CurrentQueryResult.UpperLeftIndex, MouseEvent);
		return;
	}

	if (!IsInGridBounds(ItemDropIndex, HoverItem->GetGridDimensions()))
		return;

	// 在ItemDropIndex中放置物品
	auto GridSlot = GridSlots[ItemDropIndex];
	if (!GridSlot->GetInventoryItem().IsValid())
	{
		PutDownOnIndex(ItemDropIndex);  
	}
}

void UInvInventoryGrid::PutDownOnIndex(const int32 Index)
{
	AddItemAtIndex(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount());
	UpdateGridSlots(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount()); // 更新指定Item所占据的一部分GridSlots
	ClearHoverItem();
}

void UInvInventoryGrid::ClearHoverItem()
{
	if (!IsValid(HoverItem))
		return;

	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetIsStackable(false);
	HoverItem->SetPreviousGridIndex(INDEX_NONE);
	HoverItem->UpdateStackCount(0);
	HoverItem->SetImageBrush(FSlateNoResource());

	HoverItem->RemoveFromParent(); // 为什么不直接remove？
	HoverItem = nullptr;

	ShowCursor();	
}

UUserWidget* UInvInventoryGrid::GetVisibleCursorWidget()
{
	if (!IsValid(GetOwningPlayer()))
		return nullptr;

	if (!IsValid(VisibleCursorWidget))
	{
		VisibleCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), VisibleCursorWidgetClass);
	}

	return VisibleCursorWidget;
}

UUserWidget* UInvInventoryGrid::GetHiddenCursorWidget()
{
	if (!IsValid(GetOwningPlayer()))
		return nullptr;

	if (!IsValid(HiddenCursorWidget))
	{
		HiddenCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), HiddenCursorWidgetClass);
	}

	return HiddenCursorWidget;
}

bool UInvInventoryGrid::IsSameStackable(const UInvInventoryItem* ClickedInventoryItem) const
{
	const bool bIsSameItem = ClickedInventoryItem == HoverItem->GetInventoryItem();
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsSameItem && bIsStackable && HoverItem->GetItemType().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

void UInvInventoryGrid::SwapWithHoverItem(UInvInventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	if (!IsValid(HoverItem))
		return;

	UInvInventoryItem* TempInventoryItem = HoverItem->GetInventoryItem();
	const int32 TempStackCount = HoverItem->GetStackCount();
	const bool bTempIsStackable = HoverItem->IsStackable();

	// 保持PreviousGridIndex相同
	AssignHoverItem(ClickedInventoryItem, GridIndex, HoverItem->GetPreviousGridIndex());
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
	AddItemAtIndex(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
	UpdateGridSlots(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
	
}

bool UInvInventoryGrid::ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount,
	const int32 MaxStackSize) const
{
	return RoomInClickedSlot == 0 && HoveredStackCount < MaxStackSize;
}

void UInvInventoryGrid::SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index)
{
	UInvGridSlot* GridSlot = GridSlots[Index];
	GridSlot->SetStackCount(HoveredStackCount);

	UInvSlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(HoveredStackCount);

	HoverItem->UpdateStackCount(ClickedStackCount);
}

bool UInvInventoryGrid::ShouldConsumeHoverItemStacks(const int32 HoveredStackCount, const int32 RoomInClickedSlot) const
{
	return HoveredStackCount <= RoomInClickedSlot; // 能填充，没有溢出
}

void UInvInventoryGrid::ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 Index)
{
	const int32 AmountToTransfer = HoveredStackCount;
	const int32 NewClickedStackCount = ClickedStackCount + AmountToTransfer;

	// 设置新的网格物品数量，删除悬浮物品
	GridSlots[Index]->SetStackCount(NewClickedStackCount);
	SlottedItems.FindChecked(Index)->UpdateStackCount(NewClickedStackCount);
	ClearHoverItem();
	ShowCursor();

	// 解决两个物体叠加后背景依然是灰色纹理的问题
	const FInvGridFragment* GridFragment = GridSlots[Index]->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FInvGridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	HighlightSlots(Index, Dimensions); 
}

bool UInvInventoryGrid::ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const
{
	return RoomInClickedSlot < HoveredStackCount;
}

void UInvInventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index)
{
	UInvGridSlot* GridSlot = GridSlots[Index];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;
	GridSlot->SetStackCount(NewStackCount);

	UInvSlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(NewStackCount);

	HoverItem->UpdateStackCount(Remainder);
}

void UInvInventoryGrid::CreateItemPopUp(const int32 GridIndex)
{
	UInvInventoryItem* RightClickedItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	
	if (!IsValid(RightClickedItem))
		return;

	if (IsValid(GridSlots[GridIndex]->GetItemPopUp())) // 防止多次创建
		return;

	ItemPopUp = CreateWidget<UInvItemPopUp>(this, ItemPopUpClass);
	GridSlots[GridIndex]->SetItemPopUp(ItemPopUp); // GridSlot观察ItemPopUp

	// 设置ItemPopUp参数并绘制？
	OwningCanvasPanel->AddChild(ItemPopUp);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUp);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	CanvasSlot->SetPosition(MousePosition - ItemPopUpOffset);
	CanvasSlot->SetSize(ItemPopUp->GetBoxSize());

	const int32 SliderMax = GridSlots[GridIndex]->GetStackCount() - 1;
	if (RightClickedItem->IsStackable() && SliderMax > 0) // 如果可分割
	{
		ItemPopUp->OnSplit.BindDynamic(this, &ThisClass::OnPopUpMenuSplit); // 分割回调
		ItemPopUp->SetSliderParams(SliderMax, FMath::Max(1, GridSlots[GridIndex]->GetStackCount() / 2));
	}
	else
		ItemPopUp->CollapseSplitButton();
	
	ItemPopUp->OnDrop.BindDynamic(this, &ThisClass::OnPopUpMenuDrop);
	
	if (RightClickedItem->IsConsumable())
		ItemPopUp->OnConsume.BindDynamic(this, &ThisClass::OnPopUpMenuConsume);
	else
		ItemPopUp->CollapseConsumeButton();
}

void UInvInventoryGrid::DropItem()
{
	if (!IsValid(HoverItem))
		return;

	if (!IsValid(HoverItem->GetInventoryItem()))
		return;

	InventoryComponent->Server_DropItem(HoverItem->GetInventoryItem(), HoverItem->GetStackCount());
	
	ClearHoverItem();
	ShowCursor();
}

bool UInvInventoryGrid::HasHoverItem() const
{
	return IsValid(HoverItem);
}

UInvHoverItem* UInvInventoryGrid::GetHoverItem() const
{
	return HoverItem;
}

void UInvInventoryGrid::ShowCursor()
{
	if (!IsValid(GetOwningPlayer()))
		return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Type::Default, GetVisibleCursorWidget());
}

void UInvInventoryGrid::HideCursor()
{
	if (!IsValid(GetOwningPlayer()))
		return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Type::Default, GetHiddenCursorWidget());
}

void UInvInventoryGrid::SetOwningCanvas(UCanvasPanel* OwningCanvas)
{
	OwningCanvasPanel = OwningCanvas;
}

void UInvInventoryGrid::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) // 如果 有 选中的物品，就返回
		return;

	UInvGridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetOccupiedTexture();
	}
}

void UInvInventoryGrid::OnGridSlotUnHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) // 如果 有 选中的物品，就返回
		return;
	
	UInvGridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetUnOccupiedTexture(); 
	}
}

void UInvInventoryGrid::OnPopUpMenuSplit(int32 SplitAmount, int32 Index)
{
	UInvInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem))
		return;

	if (!RightClickedItem->IsStackable())
		return;

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInvGridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 StackCount = UpperLeftGridSlot->GetStackCount();
	const int32 NewStackCount = StackCount - SplitAmount;

	// 设置GridSlot和SlottedItem的StackCount
	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	AssignHoverItem(RightClickedItem, UpperLeftIndex, UpperLeftIndex);
	HoverItem->UpdateStackCount(SplitAmount);
}

void UInvInventoryGrid::OnPopUpMenuDrop(int32 Index)
{
	UInvInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem))
		return;

	PickUp(RightClickedItem, Index);
	DropItem();
}

void UInvInventoryGrid::OnPopUpMenuConsume(int32 Index)
{
	UInvInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem))
		return;

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInvGridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 NewStackCount = UpperLeftGridSlot->GetStackCount() - 1;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	InventoryComponent->Server_ConsumeItem(RightClickedItem); // 服务器

	if (NewStackCount <= 0)
	{
		RemoveItemFromGrid(RightClickedItem, Index);
	}
}

bool UInvInventoryGrid::
MatchesCategory(const UInvInventoryItem* Item) const
{
	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}


