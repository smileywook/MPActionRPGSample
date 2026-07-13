// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "MPPlayerState.h"

namespace
{
	FString NetModeToString(const ENetMode NetMode)
	{
		switch (NetMode)
		{
		case NM_Standalone:
			return TEXT("Standalone");
		case NM_DedicatedServer:
			return TEXT("DedicatedServer");
		case NM_ListenServer:
			return TEXT("ListenServer");
		case NM_Client:
			return TEXT("Client");
		default:
			return TEXT("Unknown");
		}
	}

	FString RoleToString(const ENetRole Role)
	{
		switch (Role)
		{
		case ROLE_None:
			return TEXT("None");
		case ROLE_SimulatedProxy:
			return TEXT("SimulatedProxy");
		case ROLE_AutonomousProxy:
			return TEXT("AutonomousProxy");
		case ROLE_Authority:
			return TEXT("Authority");
		default:
			return TEXT("Unknown");
		}
	}
}

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[PlayerController BeginPlay] Name=%s | NetMode=%s | LocalRole=%s | RemoteRole=%s | HasAuthority=%s | IsLocalController=%s"),
		*GetName(),
		*NetModeToString(GetNetMode()),
		*RoleToString(GetLocalRole()),
		*RoleToString(GetRemoteRole()),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		IsLocalController() ? TEXT("true") : TEXT("false"));

	if (IsLocalController() && NetworkDebugWidgetClass)
	{
		NetworkDebugWidget = CreateWidget<UUserWidget>(this, NetworkDebugWidgetClass);

		if (NetworkDebugWidget)
		{
			NetworkDebugWidget->AddToViewport(999);
		}
	}

	TryBindPlayerStateEvents();
}

void AMPPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	TryBindPlayerStateEvents();
}

void AMPPlayerController::TryBindPlayerStateEvents()
{
	if (!IsLocalController())
	{
		return;
	}

	AMPPlayerState* MPPlayerState = GetPlayerState<AMPPlayerState>();
	if (!MPPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] TryBindPlayerStateEvents failed. PlayerState is invalid. Controller=%s"), *GetName());

		return;
	}

	if (CachedMPPlayerState == MPPlayerState)
	{
		return;
	}

	if (CachedMPPlayerState)
	{
		CachedMPPlayerState->OnPlayerDisplayNameChanged.RemoveDynamic(this, &AMPPlayerController::HandlePlayerDisplayNameChanged);
	}

	CachedMPPlayerState = MPPlayerState;
	CachedMPPlayerState->OnPlayerDisplayNameChanged.RemoveDynamic(this, &AMPPlayerController::HandlePlayerDisplayNameChanged);
	CachedMPPlayerState->OnPlayerDisplayNameChanged.AddDynamic(this, &AMPPlayerController::HandlePlayerDisplayNameChanged);

	UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Bound PlayerState events. Controller=%s | PlayerState=%s | DisplayName=%s"),
		*GetName(),
		*CachedMPPlayerState->GetName(),
		*CachedMPPlayerState->GetPlayerDisplayName());

	// 초기 동기화.
	// 이미 복제된 값이 있는 상태에서 바인딩될 수 있으므로 현재 값을 한 번 직접 반영한다.
	HandlePlayerDisplayNameChanged(CachedMPPlayerState->GetPlayerDisplayName());
}

void AMPPlayerController::HandlePlayerDisplayNameChanged(const FString& NewDisplayName)
{
	UE_LOG(LogTemp, Warning, TEXT("[PlayerController] PlayerDisplayName event received. Controller=%s | DisplayName=%s | NetMode=%s | IsLocalController=%s"),
		*GetName(),
		*NewDisplayName,
		*NetModeToString(GetNetMode()),
		IsLocalController() ? TEXT("true") : TEXT("false"));
}
