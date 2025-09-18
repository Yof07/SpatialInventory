// Types/InvGridTypes.h

#pragma once

#include "InvGridTypes.generated.h"

class UInvInventoryItem;

UENUM(BlueprintType) // 在蓝图中也可使用
enum class EInvItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None
};

USTRUCT()
struct FInvSlotAvailability
{
	GENERATED_BODY()

	FInvSlotAvailability() {}
	FInvSlotAvailability(int32 Index, int32 AmountToFill, bool bItemAtIndex) : Index(Index), AmountToFill(AmountToFill), bItemAtIndex(bItemAtIndex) {}

	int32 Index = INDEX_NONE; // 槽位在背包数组中的下标
	int32 AmountToFill = 0; // 如果想"补满"这个槽位，还需要放入的物品数量
	bool bItemAtIndex = false; // 该槽位是否放置有物品，即是否空槽
};

/**
 * 一次性查询整个背包后得到的"汇总结果"，即我拿一个物品去询问当前背包，究竟能放多少、还剩多少放不下，物品能不能堆叠等结果
 */
USTRUCT()
struct FInvSlotAvailabilityResult
{
	GENERATED_BODY()  								
					  								
	FInvSlotAvailabilityResult() {}					
					  								
	TWeakObjectPtr<UInvInventoryItem> Item;			 // 要放入的目标物品
	int32 TotalRoomToFill = 0;						 // 
	int32 Remainder = 0;							 // 剩余
	bool bStackable = false;						 // 要放入的物体是否支持堆叠
	TArray<FInvSlotAvailability> SlotAvailabilities; // 背包槽位的可用情况
};

// 瓦片象限
UENUM(BlueprintType)
enum class EInvTileQuadrant : uint8 
{
	TopLeft,		// 左上
	TopRight,		// 右上
	BottomLeft,		// 左下
	BottomRight,	// 右下
	None
};

USTRUCT(BlueprintType)
struct FInvTileParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	FIntPoint TileCoordinates = {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	int32 TileIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	EInvTileQuadrant TileQuadrant = EInvTileQuadrant::None;
};

inline bool operator==(const FInvTileParameters& Lhs, const FInvTileParameters& Rhs)
{
	return Lhs.TileCoordinates == Rhs.TileCoordinates && Lhs.TileIndex == Rhs.TileIndex	&& Lhs.TileQuadrant == Rhs.TileQuadrant;
}

USTRUCT()
struct FInvSpaceQueryResult
{
	GENERATED_BODY()

	// 如果没有物品则为true
	bool bHasSpace = false;

	// 在选择放置某个物品时，如果选择放置的位置只有一个物品，则ValidItem指向那个物品
	TWeakObjectPtr<UInvInventoryItem> ValidItem = nullptr;

	// ValidItem指向物品的左上角的索引
	int32 UpperLeftIndex = INDEX_NONE;
};




