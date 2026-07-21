// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MPMonsterCharacter.generated.h"

class UMPHealthComponent;

UCLASS()
class MPACTIONRPGSAMPLE_API AMPMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMPHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AttackCooldown = 1.5f;

public:
	// Sets default values for this character's properties
	AMPMonsterCharacter();

	UMPHealthComponent* GetHealthComponent() const;
	float GetAttackRange() const;
	float GetAttackDamage() const;
	float GetAttackCooldown() const;
	bool ApplyAttackDamageToActor(AActor* TargetActor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleDeath();

};
