// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerState.h"

#include "Net/UnrealNetwork.h"

AMPPlayerState::AMPPlayerState()
{
	PlayerDisplayName = TEXT("Unknown");
}

void AMPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPPlayerState, PlayerDisplayName);
}

void AMPPlayerState::SetPlayerDisplayName(const FString& NewDisplayName)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerState] SetPlayerDisplayName ignored on non-authority. Name=%s"), *GetName());

		return;
	}

	if (PlayerDisplayName == NewDisplayName)
	{
		return;
	}

	PlayerDisplayName = NewDisplayName;

	HandlePlayerDisplayNameChanged();
}

void AMPPlayerState::OnRep_PlayerDisplayName()
{
	HandlePlayerDisplayNameChanged();	
}

void AMPPlayerState::HandlePlayerDisplayNameChanged()
{
	UE_LOG(LogTemp, Warning, TEXT("[PlayerState] DisplayName changed. PlayerState=%s | DisplayName=%s | NetMode=%d | LocalRole=%d | HasAuthority=%s"),
		*GetName(),
		*PlayerDisplayName,
		static_cast<int32>(GetNetMode()),
		static_cast<int32>(GetLocalRole()),
		HasAuthority() ? TEXT("true") : TEXT("false"));

	OnPlayerDisplayNameChanged.Broadcast(PlayerDisplayName);
}
