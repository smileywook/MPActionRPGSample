// Fill out your copyright notice in the Description page of Project Settings.

#include "MPSkillComponent.h"
#include "MPHealthComponent.h"

// Sets default values for this component's properties
UMPSkillComponent::UMPSkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UMPSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* OwnerActor = GetOwner();
	const bool bHasAuthority = HasSkillAuthority();
	const ENetRole OwnerRole = OwnerActor ? OwnerActor->GetLocalRole() : ROLE_None;

	PrintSkillLog(FString::Printf(TEXT("BeginPlay Owner=%s Authority=%s Role=%d"), *GetNameSafe(OwnerActor), bHasAuthority ? TEXT("true") : TEXT("false"), static_cast<int32>(OwnerRole)));
	PrintSkillDataLog();
}

void UMPSkillComponent::RequestUseSkill()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		PrintSkillLog(TEXT("RequestUseSkill Rejected Reason=NoOwner"));
		return;
	}

	PrintSkillLog(FString::Printf(TEXT("RequestUseSkill Owner=%s Role=%d"), *GetNameSafe(OwnerActor), static_cast<int32>(OwnerActor->GetLocalRole())));
	ServerUseSkill();
}

void UMPSkillComponent::ServerUseSkill_Implementation()
{
	FString RejectReason;
	if (!CanUseSkill(RejectReason))
	{
		PrintSkillLog(FString::Printf(TEXT("ServerUseSkill Rejected Skill=%s Reason=%s"), *SkillData.SkillId.ToString(), *RejectReason));
		return;
	}

	StartSkillCooldown();

	PrintSkillLog(FString::Printf(TEXT("ServerUseSkill Accepted Skill=%s Owner=%s CooldownEnd=%.2f"), *SkillData.SkillId.ToString(), *GetNameSafe(GetOwner()), CooldownEndServerTime));

}

bool UMPSkillComponent::CanUseSkill(FString& OutReason) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		OutReason = TEXT("NoOwner");
		return false;
	}

	if (!OwnerActor->HasAuthority())
	{
		OutReason = TEXT("NotAuthority");
		return false;
	}

	if (!IsSkillDataValid())
	{
		OutReason = TEXT("InvalidSkillData");
		return false;
	}

	const UMPHealthComponent* HealthComponent = OwnerActor->FindComponentByClass<UMPHealthComponent>();
	if (!HealthComponent)
	{
		OutReason = TEXT("NoHealthComponent");
		return false;
	}

	if (HealthComponent->IsDead())
	{
		OutReason = TEXT("Dead");
		return false;
	}

	const float CurrentServerTime = GetCurrentServerTime();
	if (CurrentServerTime < CooldownEndServerTime)
	{
		const float RemainingCooldown = CooldownEndServerTime - CurrentServerTime;
		OutReason = FString::Printf(TEXT("Cooldown Remaining=%.2f"), RemainingCooldown);
		return false;
	}

	OutReason = TEXT("None");
	return true;
}

float UMPSkillComponent::GetCurrentServerTime() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}

void UMPSkillComponent::StartSkillCooldown()
{
	CooldownEndServerTime = GetCurrentServerTime() + SkillData.Cooldown;
}

const FMPSkillData& UMPSkillComponent::GetSkillData() const
{
	return SkillData;
}

bool UMPSkillComponent::HasSkillAuthority() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

bool UMPSkillComponent::IsSkillDataValid() const
{
	return !SkillData.SkillId.IsNone() && SkillData.Damage > 0.0f && SkillData.Range > 0.0f && SkillData.Radius > 0.0f && SkillData.Cooldown >= 0.0f;
}

void UMPSkillComponent::PrintSkillLog(const FString& Message) const
{
	if (!bDebugSkillLog)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SkillComponent] %s"), *Message);
}

void UMPSkillComponent::PrintSkillDataLog() const
{
	PrintSkillLog(FString::Printf(TEXT("SkillData Id=%s Name=%s Damage=%.1f Range=%.1f Radius=%.1f Cooldown=%.1f Valid=%s"), *SkillData.SkillId.ToString(), *SkillData.DisplayName, SkillData.Damage, SkillData.Range, SkillData.Radius, SkillData.Cooldown, IsSkillDataValid() ? TEXT("true") : TEXT("false")));
}
