// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

class AMPPlayerState;
class UMPHealthComponent;
class UMPNetworkDebugWidget;
class AMPActionRPGSampleCharacter;
class UMPSkillComponent;
class UProgressBar;
class UTextBlock;
class UMPCombatHUDWidget;

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMPNetworkDebugWidget> NetworkDebugWidgetClass;

	UPROPERTY()
	TObjectPtr<UMPNetworkDebugWidget> NetworkDebugWidget;

	UPROPERTY()
	TObjectPtr<UMPHealthComponent> BoundHealthComponent;

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMPCombatHUDWidget> CombatHUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UMPCombatHUDWidget> CombatHUDWidget;

private:
	UPROPERTY()
	TObjectPtr<AMPPlayerState> CachedMPPlayerState;

	TWeakObjectPtr<UMPSkillComponent> BoundSkillComponent;
	TWeakObjectPtr<UTextBlock> SkillCooldownText;
	TWeakObjectPtr<UProgressBar> SkillCooldownBar;
	FTimerHandle SkillCooldownUITimerHandle;

public:
	UFUNCTION(Exec)
	void TestDamage(float DamageAmount);
	UFUNCTION(Exec)
	void TestHeal(float HealAmount);
	UFUNCTION(Exec)
	void TestRespawn();

protected:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHP, float MaxHP);
	UFUNCTION()
	void HandleDeath();
	UFUNCTION()
	void HandleRespawn();

	UFUNCTION(Server, Reliable)
	void ServerRequestApplyTestDamage(float DamageAmount);
	UFUNCTION(Server, Reliable)
	void ServerRequestHeal(float HealAmount);
	UFUNCTION(Server, Reliable)
	void ServerRequestRespawn();

private:
	UFUNCTION()
	void HandlePlayerDisplayNameChanged(const FString& NewDisplayName);

	UFUNCTION()
	void HandleSkillCooldownChanged();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
		
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Pawn() override;

private:
	void TryBindPlayerStateEvents();

	void TryBindHealthComponent();
	void UnbindHealthComponent();

	void ApplyDamageToControlledPawn(float DamageAmount);	
	void HealControlledPawn(float HealAmount);
	void RequestRespawnControlledPawn();
	void RespawnControlledPawn(AMPActionRPGSampleCharacter* MPCharacter, UMPHealthComponent* HealthComponent);
	bool MovePawnToRespawnLocation(AMPActionRPGSampleCharacter* MPCharacter);

	void CreateNetworkDebugWidget();
	void RemoveNetworkDebugWidget();

	void CreateCombatHUD();
	void RemoveCombatHUD();

	void UpdateCombatHUDHealth(float CurrentHP, float MaxHP);
	void UpdateDeathDebugUI(bool bDead);

	void TryBindSkillComponent();
	void UnbindSkillComponent();

	void UpdateSkillCooldownUI();
};
