// Fill out your copyright notice in the Description page of Project Settings.


#include "MPMonsterAIController.h"
#include "MPMonsterCharacter.h"

AMPMonsterAIController::AMPMonsterAIController()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

void AMPMonsterAIController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log,
        TEXT("[MonsterAI] BeginPlay Name=%s NetMode=%d Authority=%d Pawn=%s"),
        *GetName(),
        static_cast<int32>(GetNetMode()),
        HasAuthority(),
        *GetNameSafe(GetPawn()));
}

void AMPMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    const AMPMonsterCharacter* MonsterCharacter = Cast<AMPMonsterCharacter>(InPawn);

    UE_LOG(LogTemp, Log,
        TEXT("[MonsterAI] OnPossess Controller=%s Pawn=%s Authority=%d ValidMonster=%d"),
        *GetName(),
        *GetNameSafe(InPawn),
        HasAuthority(),
        IsValid(MonsterCharacter));

    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error,
            TEXT("[MonsterAI] OnPossess unexpectedly executed without authority. Controller=%s"),
            *GetName());
    }
}