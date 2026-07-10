// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"

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
}
