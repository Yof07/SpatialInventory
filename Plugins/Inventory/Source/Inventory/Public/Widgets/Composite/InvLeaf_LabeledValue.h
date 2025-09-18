﻿// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvLeaf_LabeledValue.h
#pragma once

#include "CoreMinimal.h"
#include "InvLeaf.h"
#include "InvLeaf_LabeledValue.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvLeaf_LabeledValue : public UInvLeaf
{
	GENERATED_BODY()

public:
	void SetText_Label(const FText& Text, bool bCollapse) const;
	void SetText_Value(const FText& Text, bool bCollapse) const;

	/** 统一在设计时和运行时都生效的初始化逻辑 */
	virtual void NativePreConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Label;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Value;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 FontSize_Label {12};
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 FontSize_Value {18};
};
