// Fill out your copyright notice in the Description page of Project Settings.

#include "MPMonsterAIController.h"

#include "MPActionRPGSampleCharacter.h"
#include "Component/MPHealthComponent.h"
#include "MPMonsterCharacter.h"
#include "EngineUtils.h"
#include "Navigation/PathFollowingComponent.h"
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
        StopChasing(TEXT("InvalidPawn"));
        return;
    }

    SetCurrentTarget(FindNearestValidTarget());
    UpdateMovement();
}

void AMPMonsterAIController::UpdateMovement()
{
    if (!HasAuthority())
    {
        return;
    }

    AMPMonsterCharacter* MonsterCharacter = Cast<AMPMonsterCharacter>(GetPawn());
    AMPActionRPGSampleCharacter* Target = GetCurrentTarget();

    if (!IsValid(MonsterCharacter) || !IsValidTarget(Target))
    {
        StopChasing(TEXT("InvalidTarget"));
        return;
    }

    const UMPHealthComponent* MonsterHealthComponent = MonsterCharacter->GetHealthComponent();
    if (!IsValid(MonsterHealthComponent) || MonsterHealthComponent->IsDead())
    {
        StopChasing(TEXT("MonsterDead"));
        return;
    }

    const float AttackRange = MonsterCharacter->GetAttackRange();
    const float DistanceSquared = FVector::DistSquared2D(MonsterCharacter->GetActorLocation(), Target->GetActorLocation());

    if (DistanceSquared <= FMath::Square(AttackRange))
    {
        StopChasing(TEXT("WithinAttackRange"));
        return;
    }

    StartChasing(Target);
}

void AMPMonsterAIController::StartChasing(AMPActionRPGSampleCharacter* Target)
{
    if (bIsChasing || !IsValid(Target))
    {
        return;
    }

    const AMPMonsterCharacter* MonsterCharacter = Cast<AMPMonsterCharacter>(GetPawn());
    if (!IsValid(MonsterCharacter))
    {
        return;
    }

    const EPathFollowingRequestResult::Type MoveResult = MoveToActor(Target, MonsterCharacter->GetAttackRange(), false);

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MonsterAI] ChaseFailed Monster=%s Target=%s Reason=MoveRequestFailed"),
            *GetNameSafe(GetPawn()), *GetNameSafe(Target));
        return;
    }

    if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        StopChasing(TEXT("AlreadyAtGoal"));
        return;
    }

    bIsChasing = true;

    UE_LOG(LogTemp, Log, TEXT("[MonsterAI] ChaseStarted Monster=%s Target=%s AttackRange=%.1f"),
        *GetNameSafe(GetPawn()), *GetNameSafe(Target), MonsterCharacter->GetAttackRange());
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

void AMPMonsterAIController::StopChasing(const TCHAR* Reason)
{
    if (!bIsChasing)
    {
        return;
    }

    StopMovement();
    bIsChasing = false;

    UE_LOG(LogTemp, Log, TEXT("[MonsterAI] ChaseStopped Monster=%s Target=%s Reason=%s"),
        *GetNameSafe(GetPawn()), *GetNameSafe(CurrentTarget.Get()), Reason);
}

void AMPMonsterAIController::SetCurrentTarget(AMPActionRPGSampleCharacter* NewTarget)
{
    if (CurrentTarget.Get() == NewTarget)
    {
        return;
    }

    StopChasing(TEXT("TargetChanged"));

    const FString PreviousTargetName = GetNameSafe(CurrentTarget.Get());
    CurrentTarget = NewTarget;

    if (IsValid(NewTarget) && IsValid(GetPawn()))
    {
        const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), NewTarget->GetActorLocation());

        UE_LOG(LogTemp, Log, TEXT("[MonsterAI] TargetChanged Monster=%s Previous=%s Current=%s Distance=%.1f"),
            *GetNameSafe(GetPawn()), *PreviousTargetName, *GetNameSafe(NewTarget), Distance);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[MonsterAI] TargetCleared Monster=%s Previous=%s"),
        *GetNameSafe(GetPawn()), *PreviousTargetName);
}

void AMPMonsterAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    bIsChasing = false;

    if (Result.IsSuccess())
    {
        UE_LOG(LogTemp, Log, TEXT("[MonsterAI] MoveCompleted Monster=%s Target=%s Result=Success"),
            *GetNameSafe(GetPawn()), *GetNameSafe(CurrentTarget.Get()));
        return;
    }

    if (Result.Code != EPathFollowingResult::Aborted)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MonsterAI] MoveCompleted Monster=%s Target=%s Result=%d"),
            *GetNameSafe(GetPawn()), *GetNameSafe(CurrentTarget.Get()), static_cast<int32>(Result.Code));
    }
}

void AMPMonsterAIController::OnUnPossess()
{
    StopTargetSearch();
    StopChasing(TEXT("UnPossess"));
    SetCurrentTarget(nullptr);

    Super::OnUnPossess();
}
