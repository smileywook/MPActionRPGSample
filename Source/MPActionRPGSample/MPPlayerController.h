// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

class AMPPlayerState;
class UMPHealthComponent;
class UMPNetworkDebugWidget;

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
	UPROPERTY()
	TObjectPtr<AMPPlayerState> CachedMPPlayerState;

public:
	UFUNCTION(Exec)
	void TestDamage(float DamageAmount);
	UFUNCTION(Exec)
	void TestHeal(float HealAmount);

protected:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHP, float MaxHP);
	UFUNCTION()
	void HandleDeath();

	UFUNCTION(Server, Reliable)
	void ServerRequestApplyTestDamage(float DamageAmount);
	UFUNCTION(Server, Reliable)
	void ServerRequestHeal(float HealAmount);

private:
	UFUNCTION()
	void HandlePlayerDisplayNameChanged(const FString& NewDisplayName);
	
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

	void UpdateHealthDebugUI(float CurrentHP, float MaxHP);
	void UpdateDeathDebugUI(bool bDead);
};
