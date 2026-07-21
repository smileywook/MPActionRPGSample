// Fill out your copyright notice in the Description page of Project Settings.

#include "MPMonsterAIController.h"

#include "MPActionRPGSampleCharacter.h"
#include "Component/MPHealthComponent.h"
#include "MPMonsterCharacter.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AMPMonsterAIController::AMPMonsterAIController()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

AMPActionRPGSampleCharacter* AMPMonsterAIController::GetCurrentTarget() const
{
    return CurrentTarget.Get();
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

    if (HasAuthority() && IsValid(GetPawn()))
    {
        StartTargetSearch();
    }
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

    if (!IsValid(MonsterCharacter))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[MonsterAI] Target search rejected. Reason=InvalidMonster Pawn=%s"),
            *GetNameSafe(InPawn));
        return;
    }

    StartTargetSearch();
}

void AMPMonsterAIController::StartTargetSearch()
{
    if (!HasAuthority() || !IsValid(GetWorld()) || !IsValid(GetPawn()))
    {
        return;
    }

    StopTargetSearch();
    UpdateTarget();

    const float ValidSearchInterval = FMath::Max(TargetSearchInterval, 0.1f);
    GetWorldTimerManager().SetTimer(TargetSearchTimerHandle, this,
        &AMPMonsterAIController::UpdateTarget, ValidSearchInterval, true);

    UE_LOG(LogTemp, Log,
        TEXT("[MonsterAI] TargetSearchStarted Monster=%s Interval=%.2f Radius=%.1f"),
        *GetNameSafe(GetPawn()),
        ValidSearchInterval,
        TargetSearchRadius);
}

void AMPMonsterAIController::StopTargetSearch()
{
    if (!IsValid(GetWorld()))
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(TargetSearchTimerHandle);
}

void AMPMonsterAIController::UpdateTarget()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!IsValid(GetPawn()))
    {
        SetCurrentTarget(nullptr);
        return;
    }

    SetCurrentTarget(FindNearestValidTarget());
}

bool AMPMonsterAIController::IsValidTarget(const AMPActionRPGSampleCharacter* Candidate) const
{
    if (!IsValid(Candidate) || Candidate == GetPawn())
    {
        return false;
    }

    const UMPHealthComponent* HealthComponent = Candidate->GetHealthComponent();

    if (!IsValid(HealthComponent) || HealthComponent->IsDead())
    {
        return false;
    }

    return true;
}

AMPActionRPGSampleCharacter* AMPMonsterAIController::FindNearestValidTarget() const
{
    UWorld* World = GetWorld();
    const APawn* ControlledPawn = GetPawn();

    if (!IsValid(World) || !IsValid(ControlledPawn))
    {
        return nullptr;
    }

    AMPActionRPGSampleCharacter* NearestTarget = nullptr;
    const FVector SearchOrigin = ControlledPawn->GetActorLocation();
    float NearestDistanceSquared = FMath::Square(FMath::Max(TargetSearchRadius, 0.0f));

    for (TActorIterator<AMPActionRPGSampleCharacter> It(World); It; ++It)
    {
        AMPActionRPGSampleCharacter* Candidate = *It;

        if (!IsValidTarget(Candidate))
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(SearchOrigin, Candidate->GetActorLocation());

        if (DistanceSquared > NearestDistanceSquared)
        {
            continue;
        }

        NearestTarget = Candidate;
        NearestDistanceSquared = DistanceSquared;
    }

    return NearestTarget;
}

void AMPMonsterAIController::SetCurrentTarget(AMPActionRPGSampleCharacter* NewTarget)
{
    if (CurrentTarget.Get() == NewTarget)
    {
        return;
    }

    const FString PreviousTargetName = GetNameSafe(CurrentTarget.Get());
    CurrentTarget = NewTarget;

    if (IsValid(NewTarget) && IsValid(GetPawn()))
    {
        const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), NewTarget->GetActorLocation());

        UE_LOG(LogTemp, Log, 
            TEXT("[MonsterAI] TargetChanged Monster=%s Previous=%s Current=%s Distance=%.1f"),
            *GetNameSafe(GetPawn()),
            *PreviousTargetName,
            *GetNameSafe(NewTarget),
            Distance);
        return;
    }

    UE_LOG(LogTemp, Log,
        TEXT("[MonsterAI] TargetCleared Monster=%s Previous=%s"),
        *GetNameSafe(GetPawn()),
        *PreviousTargetName);
}

void AMPMonsterAIController::OnUnPossess()
{
    StopTargetSearch();
    SetCurrentTarget(nullptr);

    Super::OnUnPossess();
}
