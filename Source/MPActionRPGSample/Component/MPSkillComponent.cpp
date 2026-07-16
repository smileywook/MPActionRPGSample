// Fill out your copyright notice in the Description page of Project Settings.

#include "MPSkillComponent.h"
#include "MPHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

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

void UMPSkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMPSkillComponent, CooldownEndServerTime, COND_OwnerOnly);
}

void UMPSkillComponent::OnRep_CooldownEndServerTime()
{
	PrintSkillLog(FString::Printf(TEXT("OnRep_CooldownEndServerTime EndTime=%.2f Remaining=%.2f"), CooldownEndServerTime, GetRemainingCooldown()));
	NotifySkillCooldownChanged();
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
	MulticastSkillActivated();

	PrintSkillLog(FString::Printf(TEXT("ServerUseSkill Accepted Skill=%s Owner=%s CooldownEnd=%.2f"), *SkillData.SkillId.ToString(), *GetNameSafe(GetOwner()), CooldownEndServerTime));

	const int32 DamagedTargetCount = PerformSkillTrace();

	PrintSkillLog(FString::Printf(TEXT("ServerUseSkill Completed Skill=%s DamagedTargets=%d"), *SkillData.SkillId.ToString(), DamagedTargetCount));

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

void UMPSkillComponent::StartSkillCooldown()
{
	CooldownEndServerTime = GetCurrentServerTime() + SkillData.Cooldown;

	PrintSkillLog(FString::Printf(TEXT("Cooldown Started EndTime=%.2f Duration=%.2f Authority=%s"), CooldownEndServerTime, SkillData.Cooldown, HasSkillAuthority() ? TEXT("true") : TEXT("false")));

	NotifySkillCooldownChanged(); 

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
}

void UMPSkillComponent::NotifySkillCooldownChanged()
{
	PrintSkillLog(FString::Printf(TEXT("NotifySkillCooldownChanged Owner=%s Remaining=%.2f"), *GetNameSafe(GetOwner()), GetRemainingCooldown()));
	OnSkillCooldownChanged.Broadcast();
}

void UMPSkillComponent::MulticastSkillActivated_Implementation()
{
	PrintSkillLog(FString::Printf(TEXT("SkillActivated Cosmetic Owner=%s Skill=%s"), *GetNameSafe(GetOwner()), *SkillData.SkillId.ToString()));
	OnSkillActivated.Broadcast();
}

float UMPSkillComponent::GetCurrentServerTime() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	const AGameStateBase* GameState = World->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
}

float UMPSkillComponent::GetRemainingCooldown() const
{
	return FMath::Max(0.0f, CooldownEndServerTime - GetCurrentServerTime());
}

float UMPSkillComponent::GetCooldownDuration() const
{
	return SkillData.Cooldown;
}

bool UMPSkillComponent::IsSkillOnCooldown() const
{
	return GetRemainingCooldown() > KINDA_SMALL_NUMBER;
}

int32 UMPSkillComponent::PerformSkillTrace()
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();

	if (!OwnerActor || !World)
	{
		PrintSkillLog(TEXT("PerformSkillTrace Rejected Reason=InvalidOwnerOrWorld"));
		return 0;
	}

	if (!OwnerActor->HasAuthority())
	{
		PrintSkillLog(TEXT("PerformSkillTrace Rejected Reason=NotAuthority"));
		return 0;
	}

	const FVector TraceStart = OwnerActor->GetActorLocation();
	const FVector TraceEnd = TraceStart + OwnerActor->GetActorForwardVector() * SkillData.Range;
	const FCollisionShape SkillCollisionShape = FCollisionShape::MakeSphere(SkillData.Radius);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GroundSlash), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FHitResult> HitResults;
	World->SweepMultiByObjectType(HitResults, TraceStart, TraceEnd, FQuat::Identity, ObjectQueryParams, SkillCollisionShape, QueryParams);

	if (bDrawSkillTrace)
	{
		DrawDebugSphere(World, TraceStart, SkillData.Radius, 16, FColor::Yellow, false, 2.0f);
		DrawDebugSphere(World, TraceEnd, SkillData.Radius, 16, FColor::Yellow, false, 2.0f);
		DrawDebugLine(World, TraceStart, TraceEnd, FColor::Yellow, false, 2.0f, 0, 2.0f);
	}

	TSet<AActor*> ProcessedActors;
	int32 DamagedTargetCount = 0;

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitActor) || HitActor == OwnerActor || ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		ProcessedActors.Add(HitActor);

		if (!ApplySkillDamageToActor(HitActor))
		{
			continue;
		}

		++DamagedTargetCount;

		if (bDrawSkillTrace)
		{
			DrawDebugSphere(World, HitActor->GetActorLocation(), 30.0f, 12, FColor::Red, false, 2.0f);
		}
	}

	if (DamagedTargetCount == 0)
	{
		PrintSkillLog(FString::Printf(TEXT("SkillTrace Miss Start=%s End=%s Radius=%.1f"), *TraceStart.ToCompactString(), *TraceEnd.ToCompactString(), SkillData.Radius));
	}
	else
	{
		PrintSkillLog(FString::Printf(TEXT("SkillTrace Hit DamagedTargets=%d"), DamagedTargetCount));
	}

	return DamagedTargetCount;
}

bool UMPSkillComponent::ApplySkillDamageToActor(AActor* TargetActor)
{
	if (!HasSkillAuthority())
	{
		PrintSkillLog(TEXT("ApplySkillDamage Rejected Reason=NotAuthority"));
		return false;
	}

	if (!IsValid(TargetActor))
	{
		PrintSkillLog(TEXT("ApplySkillDamage Rejected Reason=InvalidTarget"));
		return false;
	}

	UMPHealthComponent* TargetHealthComponent = TargetActor->FindComponentByClass<UMPHealthComponent>();
	if (!TargetHealthComponent)
	{
		PrintSkillLog(FString::Printf(TEXT("ApplySkillDamage Skipped Target=%s Reason=NoHealthComponent"), *GetNameSafe(TargetActor)));
		return false;
	}

	if (TargetHealthComponent->IsDead())
	{
		PrintSkillLog(FString::Printf(TEXT("ApplySkillDamage Skipped Target=%s Reason=DeadTarget"), *GetNameSafe(TargetActor)));
		return false;
	}

	TargetHealthComponent->ApplyDamage(SkillData.Damage);

	PrintSkillLog(FString::Printf(TEXT("ApplySkillDamage Target=%s Damage=%.1f"), *GetNameSafe(TargetActor), SkillData.Damage));

	return true;
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
