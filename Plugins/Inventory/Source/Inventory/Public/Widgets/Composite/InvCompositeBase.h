// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvCompositeBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "InvCompositeBase.generated.h"

/**
 * 
 */

UCLASS()
class INVENTORY_API UInvCompositeBase : public UUserWidget
{
	GENERATED_BODY()

public:
	FGameplayTag GetFragmentTag() const { return FragmentTag; }
	void SetFragmentTag(const FGameplayTag& Tag) { FragmentTag = Tag; }

	virtual void Collapse();
	/** 展开 */
	void Expand();

	using FuncType = TFunction<void(UInvCompositeBase*)>;
	virtual void ApplyFunction(FuncType Function) {}

private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag FragmentTag;
};















