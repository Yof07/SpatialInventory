// Fill out your copyright notice in the Description page of Project Settings.
// Interaction/InvHighlightableStaticMesh.h
#pragma once

#include "CoreMinimal.h"
#include "InvHighlightable.h"
#include "Components/StaticMeshComponent.h"
#include "InvHighlightableStaticMesh.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInvHighlightableStaticMesh : public UStaticMeshComponent, public IInvHighlightable
{
	GENERATED_BODY()

public:
	// 自动生成的接口实现

	/** 高亮，方法为把材质设置为高亮材质 */
	virtual void Highlight_Implementation() override;

	/** 取消高亮，方法为把材质设置为空 */
	virtual void UnHighlight_Implementation() override;

private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	TObjectPtr<UMaterialInterface> HighlightMaterial;
};
