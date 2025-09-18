// InvProxyMesh.cpp - 完整修复版

#include "EquipmentManagement/ProxyMesh/InvProxyMesh.h"
#include "EquipmentManagement/Component/InvEquipmentComponent.h"
#include "GameFramework/Character.h"

AInvProxyMesh::AInvProxyMesh()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = false;
    SetReplicates(false);

    SetFlags(RF_Transient);

    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetIsReplicated(false);

    EquipmentComponent = CreateDefaultSubobject<UInvEquipmentComponent>("EquipmentComponent");
    EquipmentComponent->SetOwningSkeletalMesh(Mesh);
    EquipmentComponent->SetIsProxy(true);
    EquipmentComponent->SetIsReplicated(false);
}

void AInvProxyMesh::BeginPlay()
{
    Super::BeginPlay();
    
    // 重要：直接使用 Owner 而不是查找
    InitializeWithOwner();
}

void AInvProxyMesh::InitializeWithOwner()
{
    // 直接使用已设置的 Owner
    APlayerController* OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
    {
        UE_LOG(LogTemp, Error, 
            TEXT("ProxyMesh %s: Owner is not a PlayerController!"), 
            *GetName());
        
        // 延迟重试
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &ThisClass::InitializeWithOwner);
        GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
        return;
    }

    // 验证是本地控制器
    if (!OwnerPC->IsLocalController())
    {
        UE_LOG(LogTemp, Error, 
            TEXT("ProxyMesh %s: Owner PC %s is not local controller!"), 
            *GetName(), *OwnerPC->GetName());
        return;
    }

    // 获取 Pawn
    ACharacter* Character = Cast<ACharacter>(OwnerPC->GetPawn());
    if (!IsValid(Character))
    {
        // Pawn 可能还没准备好，延迟重试
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &ThisClass::InitializeWithOwner);
        GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
        return;
    }

    USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
    if (!IsValid(CharacterMesh))
    {
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &ThisClass::InitializeWithOwner);
        GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
        return;
    }

    // 设置 Mesh
    SourceMesh = CharacterMesh;
    if (IsValid(SourceMesh.Get()))
    {
        Mesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());

        if (UAnimInstance* AnimInst = SourceMesh->GetAnimInstance())
        {
            Mesh->SetAnimInstanceClass(AnimInst->GetClass());
        }
        else
        {
            Mesh->SetAnimInstanceClass(nullptr);
        }
    }

    Mesh->SetIsReplicated(false);

    // 初始化 EquipmentComponent
    if (IsValid(EquipmentComponent))
    {
        EquipmentComponent->InitializeOwner(OwnerPC);
    }

    UE_LOG(LogTemp, Log, 
        TEXT("ProxyMesh %s: Successfully initialized for PC %s"), 
        *GetName(), *OwnerPC->GetName());
}
