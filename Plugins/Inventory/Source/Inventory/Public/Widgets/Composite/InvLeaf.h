// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvLeaf.h
#pragma once

#include "CoreMinimal.h"
#include "InvCompositeBase.h"
#include "InvLeaf.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInvLeaf : public UInvCompositeBase
{
	GENERATED_BODY()

public:
	virtual void ApplyFunction(FuncType Function) override;
};
