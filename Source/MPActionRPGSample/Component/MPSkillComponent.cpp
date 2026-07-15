// Fill out your copyright notice in the Description page of Project Settings.


#include "MPSkillComponent.h"

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
}

bool UMPSkillComponent::HasSkillAuthority() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

void UMPSkillComponent::PrintSkillLog(const FString& Message) const
{
	if (!bDebugSkillLog)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SkillComponent] %s"), *Message);
}