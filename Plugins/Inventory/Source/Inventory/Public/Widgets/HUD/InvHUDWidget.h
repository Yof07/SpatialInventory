// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/HUD/InvHUDWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvHUDWidget.generated.h"

class UInvInfoMessage;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	
	/**
	 * BlueprintImplementableEvent表示声明一个在蓝图中实现的函数
	 * @param Message 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void ShowPickUpMessage(const FString& Message);

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void HidePickUpMessage();

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UInvInfoMessage> InfoMessage;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	FText InfoMessageText;

	// 没有空间委托的回调
	UFUNCTION() 
	void OnNoRoom();
};
