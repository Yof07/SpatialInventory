// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Utils/InvWidgetUtils.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvWidgetUtils.generated.h"

class UWidget;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvWidgetUtils : public UBlueprintFunctionLibrary
{	
	GENERATED_BODY()
	
public:
	static int32 GetIndexFromPosition(const FIntPoint& Position, const int32 Columns); // FIntPoint：一对整数
	
	static FIntPoint GetPositionFromIndex(const int32 Index, const int32 Columns);

	// 把widget相对于父容器的坐标转换成相对于视口的坐标，坐标是指左上角坐标
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static FVector2D GetWidgetPosition(UWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool IsWithinBounds(const FVector2D& BoundaryPos, const FVector2D& WidgetSize, const FVector2D& MousePos);
	
	/**
	 * 注意要让widget进入布局，如canvasPanel要wrap with overlay
	 * @param Widget 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static FVector2D GetWidgetSize(UWidget* Widget);

	static FVector2D GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize, const FVector2D& MousePos);
};
