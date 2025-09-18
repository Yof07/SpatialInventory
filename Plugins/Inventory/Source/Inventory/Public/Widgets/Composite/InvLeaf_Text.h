// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvLeaf_Text.h
#pragma once

#include "CoreMinimal.h"
#include "InvLeaf.h"
#include "InvLeaf_Text.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvLeaf_Text : public UInvLeaf
{
	GENERATED_BODY()

public:
	void SetText(const FText& Text) const;

	/** 在编辑器里可能被多次调用（每次属性改变都会触发），运行时只在 Construct 之前调用一次 */
	virtual void NativePreConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LeafText;

	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 FontSize {12};
};
