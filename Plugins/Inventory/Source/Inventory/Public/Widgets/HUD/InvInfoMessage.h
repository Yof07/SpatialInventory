// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/HUD/InvInfoMessage.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvInfoMessage.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvInfoMessage : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	// 文字渐显
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void MessageShow();

	// 文字渐隐
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void MessageHide();

	void SetMessage(const FText& Message);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Message;

	UPROPERTY(EditAnywhere, Category="Inventory")
	float MessageLifetime = 3.f;

	FTimerHandle MessageTimer;
	
	bool bIsMessageActive = false;
};
