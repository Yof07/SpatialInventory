// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/ItemDescription/InvItemDescription.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Composite/InvComposite.h"
#include "InvItemDescription.generated.h"

class USizeBox;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvItemDescription : public UInvComposite
{
	GENERATED_BODY()

public:
	FVector2D GetBoxSize() const;

	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox;
	
};
