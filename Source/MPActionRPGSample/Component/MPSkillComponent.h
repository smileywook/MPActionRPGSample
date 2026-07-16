// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MPSkillComponent.generated.h"

USTRUCT(BlueprintType)
struct FMPSkillData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FName SkillId = FName(TEXT("GroundSlash"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FString DisplayName = TEXT("Ground Slash");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	float Damage = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	float Range = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	float Radius = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	float Cooldown = 3.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPACTIONRPGSAMPLE_API UMPSkillComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FMPSkillData SkillData;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	bool bDebugSkillLog = true;

private:
	float CooldownEndServerTime = 0.0f;

public:	
	// Sets default values for this component's properties
	UMPSkillComponent();

	void RequestUseSkill();
	const FMPSkillData& GetSkillData() const;
	bool HasSkillAuthority() const;
	bool IsSkillDataValid() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION(Server, Reliable)
	void ServerUseSkill();

	bool CanUseSkill(FString& OutReason) const;
	void StartSkillCooldown();
	float GetCurrentServerTime() const;
	void PrintSkillLog(const FString& Message) const;
	void PrintSkillDataLog() const;
};
