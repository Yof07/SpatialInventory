// InvCharacterDisplay.cpp - 最终修复版
#include "Widgets/CharacterDisplay/InvCharacterDisplay.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "EquipmentManagement/ProxyMesh/InvProxyMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Player/InvPlayerController.h"

FReply UInvCharacterDisplay::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    CurrentPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
    LastPosition = CurrentPosition;
    
    bIsDragging = true;
    return FReply::Handled();
}

FReply UInvCharacterDisplay::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    bIsDragging = false;
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UInvCharacterDisplay::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    bIsDragging = false;
}

void UInvCharacterDisplay::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    
    Mesh = nullptr;
    ProxyMeshActor = nullptr;
}

void UInvCharacterDisplay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 如果还没绑定 ProxyMeshActor，尝试获取
    if (!ProxyMeshActor.IsValid())
    {
        // 直接从 PlayerController 获取（推荐）
        AInvPlayerController* InvPC = Cast<AInvPlayerController>(GetOwningPlayer());
        if (IsValid(InvPC))
        {
            // 确保是本地控制器
            if (!InvPC->IsLocalController())
            {
                UE_LOG(LogTemp, Warning, 
                    TEXT("CharacterDisplay: OwningPlayer %s is not local controller"), 
                    *InvPC->GetName());
                return;
            }

            AInvProxyMesh* LocalProxyMesh = InvPC->GetLocalProxyMesh();
            if (IsValid(LocalProxyMesh))
            {
                ProxyMeshActor = LocalProxyMesh;
                Mesh = ProxyMeshActor->GetMesh();
                
                UE_LOG(LogTemp, Warning, 
                    TEXT("CharacterDisplay: Successfully bound to ProxyMesh %s for PC %s"), 
                    *LocalProxyMesh->GetName(),
                    *InvPC->GetName());
            }
            else
            {
                // ProxyMesh 可能还没创建，等待下一帧
                return;
            }
        }
    }

    // 如果 Mesh 还没找到，直接 return
    if (!Mesh.IsValid())
        return;

    // 拖拽旋转逻辑
    if (!bIsDragging)
        return;

    LastPosition = CurrentPosition;
    CurrentPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

    const float HorizontalDelta = LastPosition.X - CurrentPosition.X;
    Mesh->AddRelativeRotation(FRotator(0.f, HorizontalDelta, 0.f));
}