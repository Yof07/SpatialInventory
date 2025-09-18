// Fill out your copyright notice in the Description page of Project Settings.
// InventoryManagement/Utils/InvInventoryStatics.h
#pragma once

#include "CoreMinimal.h"
#include "Items/InvInventoryItem.h"
#include "Items/Components/InvItemComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/InvGridTypes.h"
#include "Widgets/Utils/InvWidgetUtils.h"
#include "InvInventoryStatics.generated.h"

class UInvInventoryBase;
class UInvHoverItem;
class UInvInventoryComponent;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvInventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "InvInventory")
	static UInvInventoryComponent* GetInvInventoryComponent(const APlayerController* PlayerController);

	static EInvItemCategory GetItemCategoryFromItemComp(UInvItemComponent* ItemComp);

	template<typename T, typename FuncT>
	static void ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function);

	UFUNCTION(BlueprintCallable, Category = "InvInventory")
	static void ItemHovered(APlayerController* PC, UInvInventoryItem* Item);

	UFUNCTION(BlueprintCallable, Category = "InvInventory")
	static void ItemUnhovered(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "InvInventory")
	static UInvHoverItem* GetHoverItem(APlayerController* PC);

	static UInvInventoryBase* GetInventoryWidget(APlayerController* PC);
};

// Function必须接收数组的元素作为参数
template <typename T, typename FuncT>
void UInvInventoryStatics::ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function)
{	
	/**
	 * Range2D是要遍历的矩形范围，Range2D.X = 宽度(列数)，Range2D.Y = 高度(行数)；
	 * Index是Range2D所代表的矩形的左上角索引
	 */
	
	// 遍历以UInvWidgetUtils::GetPositionFromIndex(Index, GridColumns)为最左上角的格子的矩形网格，获取这些矩形网格的每一个格子的坐标，然后调用Function
	for (int32 j = 0; j < Range2D.Y; j++) // 遍历行偏移
	{
		for (int32 i = 0; i < Range2D.X; i++) // 遍历列偏移
		{
			const FIntPoint Coordinates = UInvWidgetUtils::GetPositionFromIndex(Index, GridColumns) + FIntPoint(i, j);
			const int32 TileIndex = UInvWidgetUtils::GetIndexFromPosition(Coordinates, GridColumns);
			if (Array.IsValidIndex(TileIndex))
			{
				Function(Array[TileIndex]);
			}
		}
	}
	
}
