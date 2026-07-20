// Fill out your copyright notice in the Description page of Project Settings.

#include "MPMonsterCharacter.h"
#include "Component/MPHealthComponent.h"
#include "MPMonsterAIController.h"

// Sets default values
AMPMonsterCharacter::AMPMonsterCharacter()
{
    bReplicates = true;
    SetReplicateMovement(true);

    HealthComponent = CreateDefaultSubobject<UMPHealthComponent>(TEXT("HealthComponent"));

    AIControllerClass = AMPMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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

