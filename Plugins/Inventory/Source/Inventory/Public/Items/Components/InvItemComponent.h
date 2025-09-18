// Fill out your copyright notice in the Description page of Project Settings.
// Items/Components/InvItemComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Manifest/InvItemManifest.h"
#include "InvItemComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable) // Blueprintable让其能派生为蓝图
class INVENTORY_API UInvItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInvItemComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	const FInvItemManifest& GetItemManifest() const { return ItemManifest; }

	// 用来提供非常引的ItemManifest来调用ItemManifest中的非常引方法，返回类型是值而不是引用是为了保持封装
	FInvItemManifest GetItemManifest()  { return ItemManifest; }
	
	const FString& GetPickUpMessage() const {return PickUpMessage;}

	/** 调用拾取回调并销毁被拾取物品 */
	void PickedUp();

	void InitItemManifest(FInvItemManifest CopyOfManifest);

protected:
	/** 拾取表现、如声音、特效等 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnPickedUp();
	
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickUpMessage;

	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FInvItemManifest ItemManifest;
};
