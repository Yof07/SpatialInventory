// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/CharacterDisplay/InvCharacterDisplay.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvCharacterDisplay.generated.h"

/**
 * 
 */

class AInvProxyMesh;

UCLASS()
class INVENTORY_API UInvCharacterDisplay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	bool bIsDragging = false;
	
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;
	
	FVector2D CurrentPosition;
	FVector2D LastPosition;
	
	TWeakObjectPtr<AInvProxyMesh> ProxyMeshActor;
};
