// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MPSkillComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillCooldownChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillActivated);

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
	float Cooldown = 2.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPACTIONRPGSAMPLE_API UMPSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillCooldownChanged OnSkillCooldownChanged;

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillActivated OnSkillActivated;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FMPSkillData SkillData;
	
	UPROPERTY(EditDefaultsOnly, Category="Skill|Debug")
	bool bDrawSkillTrace = true;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Debug")
	bool bDebugSkillLog = true;

private:
	UPROPERTY(ReplicatedUsing = OnRep_CooldownEndServerTime)
	float CooldownEndServerTime = 0.0f;

public:	
	// Sets default values for this component's properties
	UMPSkillComponent();

	void RequestUseSkill();
	const FMPSkillData& GetSkillData() const;
	bool HasSkillAuthority() const;
	bool IsSkillDataValid() const;

	float GetRemainingCooldown() const;
	float GetCooldownDuration() const;
	bool IsSkillOnCooldown() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_CooldownEndServerTime();

	UFUNCTION(Server, Reliable)
	void ServerUseSkill();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSkillActivated();

	bool CanUseSkill(FString& OutReason) const;
	void StartSkillCooldown();
	void NotifySkillCooldownChanged();
	float GetCurrentServerTime() const;
	int32 PerformSkillTrace();
	bool ApplySkillDamageToActor(AActor* TargetActor);
	void PrintSkillLog(const FString& Message) const;
	void PrintSkillDataLog() const;
};
