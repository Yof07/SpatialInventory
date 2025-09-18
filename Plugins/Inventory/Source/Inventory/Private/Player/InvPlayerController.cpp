// Fill out your copyright notice in the Description page of Project Settings.
// Player/InvPlayerController.cpp

#include "Player/InvPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" // 要在Inventory.Build.cs添加对增强输入模块的依赖
#include "EquipmentManagement/ProxyMesh/InvProxyMesh.h"
#include "GameFramework/PlayerState.h"
#include "Interaction/InvHighlightable.h"
#include "InventoryManagement/Components/InvInventoryComponent.h"
#include "Items/Components/InvItemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/HUD/InvHUDWidget.h"


AInvPlayerController::AInvPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	TraceLength = 500.f;
}

void AInvPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	TraceForItem();
}

void AInvPlayerController::ToggleInventory()
{
	if (!InventoryComponent.IsValid()) // 使用所指针前先检查
		return;

	InventoryComponent->ToggleInventoryMenu();

	// 打开背包隐藏白色中心点
	if (InventoryComponent->IsMenuOpen())
	{
		HUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		HUDWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void AInvPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!IsValid(Subsystem))
	{
		UE_LOG(LogInput, Warning, TEXT("EnhancedInputLocalPlayerSubsystem is invalid")); // 要在Inventory.Build.cs中添加对InputCore的依赖
		return;
	}
	Subsystem->AddMappingContext(DefaultIMC, 0);

	CreateLocalProxyMesh();
	
	// 检测InvPlayerController的蓝图子类中是否手动挂载了InventoryComponent并获取
	InventoryComponent = FindComponentByClass<UInvInventoryComponent>(); 
	
	CreateHUDWidget(); // 创建屏幕中心点，中心点和物品重合时，会出现交互视图
}

void AInvPlayerController::SetupInputComponent()
{	
	Super::SetupInputComponent();
	
	// 将其转化为增强输入组件以便将输入动作和回调绑定
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent); // InputComponent是继承自Actor的字段
	
	/**
	 * ETriggerEvent::Started：表示只在摁键首次摁下时调用回调
	 * this：调用回调的对象
	 * &ThisClass::PrimaryInteract：成员函数指针
	 */
	EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &ThisClass::PrimaryInteract);
	EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &ThisClass::ToggleInventory); 
}

void AInvPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupLocalProxyMesh();
	
	Super::EndPlay(EndPlayReason);
}

void AInvPlayerController::PrimaryInteract()
{
	// 射线是否识别到物体
	if (!CurrentActor.IsValid())
		return;

	// 判断该物体是否可拾取
	UInvItemComponent* ItemComponent = CurrentActor->FindComponentByClass<UInvItemComponent>(); // 
	if (!IsValid(ItemComponent) || !InventoryComponent.IsValid())
		return;
	
	// 尝试把物品添加到库存中
	InventoryComponent->TryAddItem(ItemComponent);
}	

void AInvPlayerController::CreateHUDWidget()
{
	if (!IsLocalController()) // 防止非本地实例创建UI，浪费性能
		return;
	
	// 未填蓝图类
	if (!HUDWidgetClass) // IsValid检查Uobject对象实例，TSubclassOf是类引用模板
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: HUDWidgetClass未设置，无法创建"), *GetName());
		return;
	}
	
	// OwnerType是UObject
	HUDWidget = CreateWidget<UInvHUDWidget>(this, HUDWidgetClass);

	if (!IsValid(HUDWidget)) // 会隐式调用Get获取裸指针
	{
		UE_LOG(LogTemp, Warning, TEXT("%s 创建失败"), *GetName());
		return;
	}

	HUDWidget->AddToViewport();
}

void AInvPlayerController::TraceForItem()
{	
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport))
		return;
	
	FVector TraceStart; // 射线起点，摄像机位置
	FVector Forward; // 射线方向，即摄像机指向屏幕点(ViewportCenter)的单位向量
	
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D ViewportCenter = ViewportSize / 2.0; // 视口中心坐标
	
	// 将屏幕坐标转化为世界坐标，获取射线起点和射线方向
	if (!UGameplayStatics::DeprojectScreenToWorld(this, ViewportCenter, TraceStart, Forward))
		return;
	
	const FVector TraceEnd = TraceStart + Forward * TraceLength; // 计算射线终点
	
	FHitResult HitResult;	
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ItemTraceChannel); // 射线检测，single表示返回第一个碰撞点信息
	
	/**
	 * 从有碰撞到无碰撞，则LastActor有效；从无碰撞到有碰撞，则LastActor有效；若一直有碰撞或无碰撞，则结果相等
	 * "无碰撞到有碰撞"表现为鼠标移动到可交互物体，"有碰撞到无碰撞"表现为鼠标从可交互物体中移开
	 */ 
	LastActor = CurrentActor;
	CurrentActor = HitResult.GetActor(); // 射线检测到的第一个Actor
	
	// 射线没有检测到物体
	if (!CurrentActor.IsValid())
	{
		if (IsValid(HUDWidget))
			HUDWidget->HidePickUpMessage();
	}

	/** 核心思想：状态变化检测，仅在前后状态发生变化的时候进行复杂的操作 */
	if (CurrentActor == LastActor)
		return;

	// 判断是否可高亮并尝试高亮，判断是否可拾取并展示交互面板
	if (CurrentActor.IsValid()) // 弱指针知道自己指向的对象什么时候"有效"
	{
		// 尝试获取拥有高亮接口的组件，并显示高亮
		if (UActorComponent* Highlightable = CurrentActor->FindComponentByInterface(UInvHighlightable::StaticClass()); IsValid(Highlightable))
		{
			IInvHighlightable::Execute_Highlight(Highlightable);
		}

		// 通过判断是否拥有ItemComponent来判断是否可交互
		UInvItemComponent* ItemComponent = CurrentActor->FindComponentByClass<UInvItemComponent>(); // 动态查找UInvItemComponent组件来获取PickUpMessage
		if (!IsValid(ItemComponent))
			return;

		// 显示交互组件
		if (IsValid(HUDWidget))
			HUDWidget->ShowPickUpMessage(ItemComponent->GetPickUpMessage());
	}		

	// 取消高亮
	if (LastActor.IsValid())
	{
		// 获取拥有接口的组件，取消高亮
		if (UActorComponent* Highlightable = LastActor->FindComponentByInterface(UInvHighlightable::StaticClass()); IsValid(Highlightable))
		{
			IInvHighlightable::Execute_UnHighlight(Highlightable);
		}
	}
}

// InvPlayerController::CreateLocalProxyMesh - 验证版
void AInvPlayerController::CreateLocalProxyMesh()
{
    // 确保只在本地客户端创建
    if (!IsLocalController())
    {
        UE_LOG(LogTemp, Warning, 
            TEXT("PlayerController %s: Not local controller, skipping ProxyMesh creation"), 
            *GetName());
        return;
    }

    if (!IsValid(ProxyMeshClass))
    {
        UE_LOG(LogTemp, Warning, 
            TEXT("PlayerController %s: ProxyMeshClass not set"), 
            *GetName());
        return;
    }

    // 清理任何现有的 ProxyMesh
    CleanupLocalProxyMesh();

    FActorSpawnParameters Params;
    Params.Owner = this;  // 设置 Owner 为当前 PlayerController
    Params.Instigator = GetPawn();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    // 使用包含 PlayerController 名称的唯一名称
    FString UniqueName = FString::Printf(TEXT("ProxyMesh_%s_%d"), 
        *GetName(), 
        FMath::RandRange(1000, 9999));
    Params.Name = FName(*UniqueName);
    
    // 在远处创建，避免与游戏世界干扰
    FVector HiddenLocation = FVector(1e6f, 1e6f, 1e6f);
    
    LocalProxyMesh = GetWorld()->SpawnActor<AInvProxyMesh>(
        ProxyMeshClass, 
        HiddenLocation, 
        FRotator::ZeroRotator, 
        Params);
    
    if (!LocalProxyMesh.IsValid())
    {
        UE_LOG(LogTemp, Warning, 
            TEXT("PlayerController %s: Failed to spawn ProxyMesh"), 
            *GetName());
        return;
    }

    // 验证 Owner 设置正确
    if (LocalProxyMesh->GetOwner() != this)
    {
        UE_LOG(LogTemp, Warning, 
            TEXT("PlayerController %s: ProxyMesh owner is not this controller! Actual owner: %s"), 
            *GetName(),
            LocalProxyMesh->GetOwner() ? *LocalProxyMesh->GetOwner()->GetName() : TEXT("NULL"));
        
        // 尝试手动设置 Owner
        LocalProxyMesh->SetOwner(this);
    }

    // 确保设置为非复制
    LocalProxyMesh->SetReplicates(false);
    LocalProxyMesh->SetReplicateMovement(false);
    LocalProxyMesh->SetActorEnableCollision(false);
    LocalProxyMesh->SetFlags(RF_Transient);

    UE_LOG(LogTemp, Warning, 
        TEXT("PlayerController %s: Successfully created ProxyMesh %s with Owner %s"), 
        *GetName(), 
        *LocalProxyMesh->GetName(),
        *LocalProxyMesh->GetOwner()->GetName());
}

void AInvPlayerController::CleanupLocalProxyMesh()
{
	if (LocalProxyMesh.IsValid())
	{
		LocalProxyMesh->Destroy();
		LocalProxyMesh = nullptr;
	}
}

