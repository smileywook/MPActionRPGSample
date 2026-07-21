// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MPMonsterAIController.generated.h"

class AMPActionRPGSampleCharacter;
class UMPHealthComponent;

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API AMPMonsterAIController : public AAIController
{
	GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly, Category = "AI|Target", meta = (ClampMin = "0.1"))
    float TargetSearchInterval = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Target", meta = (ClampMin = "0.0"))
    float TargetSearchRadius = 2000.0f;

    UPROPERTY(Transient)
    TWeakObjectPtr<AMPActionRPGSampleCharacter> CurrentTarget;

    UPROPERTY(Transient)
    TWeakObjectPtr<UMPHealthComponent> BoundMonsterHealthComponent;

    bool bIsChasing = false;
    float NextAttackServerTime = 0.0f;
    FTimerHandle TargetSearchTimerHandle;

public:
	AMPMonsterAIController();

    AMPActionRPGSampleCharacter* GetCurrentTarget() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
    void StartTargetSearch();
    void StopTargetSearch();
    void UpdateTarget();
    void UpdateMovement();
    void StartChasing(AMPActionRPGSampleCharacter* Target);
    void StopChasing(const TCHAR* Reason);
    void TryAttackTarget(AMPActionRPGSampleCharacter* Target);
    bool CanAttackTarget(const AMPActionRPGSampleCharacter* Target) const;
    AMPActionRPGSampleCharacter* FindNearestValidTarget() const;
    bool IsValidTarget(const AMPActionRPGSampleCharacter* Candidate) const;
    void SetCurrentTarget(AMPActionRPGSampleCharacter* NewTarget);
    void BindMonsterHealthComponent(UMPHealthComponent* HealthComponent);
    void UnbindMonsterHealthComponent();

    UFUNCTION()
    void HandleMonsterDeath();
	
};
