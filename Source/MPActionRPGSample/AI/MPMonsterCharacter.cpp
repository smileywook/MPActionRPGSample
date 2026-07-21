// Fill out your copyright notice in the Description page of Project Settings.

#include "MPMonsterCharacter.h"
#include "Component/MPHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPMonsterAIController.h"

// Sets default values
AMPMonsterCharacter::AMPMonsterCharacter()
{
    bReplicates = true;
    SetReplicateMovement(true);

    HealthComponent = CreateDefaultSubobject<UMPHealthComponent>(TEXT("HealthComponent"));

    AIControllerClass = AMPMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}

// Called when the game starts or when spawned
void AMPMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    UE_LOG(LogTemp, Log,
        TEXT("[Monster] BeginPlay Name=%s NetMode=%d LocalRole=%d RemoteRole=%d Authority=%d Controller=%s HealthComponent=%s"),
        *GetName(),
        static_cast<int32>(GetNetMode()),
        static_cast<int32>(GetLocalRole()),
        static_cast<int32>(GetRemoteRole()),
        HasAuthority(),
        *GetNameSafe(GetController()),
        *GetNameSafe(HealthComponent));
}

UMPHealthComponent* AMPMonsterCharacter::GetHealthComponent() const
{
    return HealthComponent;
}

float AMPMonsterCharacter::GetAttackRange() const
{
    return AttackRange;
}

