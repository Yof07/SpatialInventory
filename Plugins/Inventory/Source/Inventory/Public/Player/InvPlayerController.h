// Fill out your copyright notice in the Description page of Project Settings.
// Player/InvPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InvPlayerController.generated.h"

class AInvProxyMesh;

class UInvInventoryComponent;
class UInvHUDWidget;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class INVENTORY_API AInvPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AInvPlayerController();

	virtual void Tick(float DeltaSeconds) override;

	/**
	 * 绑定于ToggleInventoryAction的回调
	 */
	UFUNCTION(BlueprintCallable)
	void ToggleInventory();

	AInvProxyMesh* GetLocalProxyMesh() const { return LocalProxyMesh.IsValid() ? LocalProxyMesh.Get() : nullptr; }
	
protected:
	virtual void BeginPlay() override;

	/** 绑定增强组件的输入动作回调 */
	virtual void SetupInputComponent() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** 绑定于PrimaryInteractAction的回调，交互逻辑 */
	void PrimaryInteract();
	
	/** 创建屏幕中心点 */
	void CreateHUDWidget();

	/** 识别可拾取的物体，存储到LastActor和CurrentActor、对其边缘显示高亮并显示交互框 */
	void TraceForItem();

	void CreateLocalProxyMesh();

	void CleanupLocalProxyMesh();
	
private:
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	/** 拾取物品 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;
 
	/** 背包打开关闭 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	/** 指定要创建的HUD类 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInvHUDWidget> HUDWidgetClass;

	/** 中心点 */
	UPROPERTY()
	TObjectPtr<UInvHUDWidget> HUDWidget;
	
	/** 射线长度 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	double TraceLength;

	/**
	 * 反射出来的枚举属性在序列化、蓝图层面只支持uint8，而ECollisionChannel是32位
	 * 检测被拾取物体的射线检测通道
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;

	/** 保证指针不会被序列化？ */
	UPROPERTY(Transient, DuplicateTransient)
	TWeakObjectPtr<AInvProxyMesh> LocalProxyMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<AInvProxyMesh> ProxyMeshClass;
	
	// TWeakObjectPtr用于"观察"目标对象，不会阻止它被销毁或回收，缓存对象可使用TWeakObjectPtr
	TWeakObjectPtr<AActor> LastActor;
	TWeakObjectPtr<AActor> CurrentActor;

	// InventoryComponent是在蓝图中附加到InvPlayerController的
	TWeakObjectPtr<UInvInventoryComponent> InventoryComponent;
};
