// Fill out your copyright notice in the Description page of Project Settings.
// EquipmentComponent/ProxyMesh/InvProxyMesh.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InvProxyMesh.generated.h"

class AInvPlayerController;
class UInvEquipmentComponent;
/**
 * 
 */
UCLASS()
class INVENTORY_API AInvProxyMesh : public AActor
{
	GENERATED_BODY()

public:
	AInvProxyMesh();
	USkeletalMeshComponent* GetMesh() const { return Mesh; }

protected:
	virtual void BeginPlay() override;

private:
	void InitializeWithOwner();
	
private:
	/** 预览 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInvEquipmentComponent> EquipmentComponent;
	
	/** 玩家控制 */ 
	TWeakObjectPtr<USkeletalMeshComponent> SourceMesh;
};	
