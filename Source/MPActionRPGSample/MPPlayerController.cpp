// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"
#include "MPActionRPGSampleCharacter.h"
#include "Component/MPHealthComponent.h"
#include "UI/MPNetworkDebugWidget.h"
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

	if (IsLocalController() && NetworkDebugWidgetClass)
	{
		NetworkDebugWidget = CreateWidget<UMPNetworkDebugWidget>(this, NetworkDebugWidgetClass);

		if (NetworkDebugWidget)
		{
			NetworkDebugWidget->AddToViewport();
		}
	}

	TryBindPlayerStateEvents();
}

void AMPPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHealthComponent();

	Super::EndPlay(EndPlayReason);
}

void AMPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	TryBindHealthComponent();
}

void AMPPlayerController::OnUnPossess()
{
	UnbindHealthComponent();

	Super::OnUnPossess();
}

void AMPPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서는 BeginPlay 시점에 PlayerState가 아직 준비되지 않을 수 있으므로
	// PlayerState 복제 완료 시점에도 바인딩을 다시 시도한다.
	TryBindPlayerStateEvents();

	TryBindHealthComponent();
}

void AMPPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	TryBindHealthComponent();
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

void AMPPlayerController::TryBindHealthComponent()
{
	if (!IsLocalController())
	{
		return;
	}

	AMPActionRPGSampleCharacter* MPCharacter = Cast<AMPActionRPGSampleCharacter>(GetPawn());
	if (!MPCharacter)
	{
		return;
	}

	UMPHealthComponent* HealthComponent = MPCharacter->GetHealthComponent();
	if (!HealthComponent)
	{
		return;
	}

	if (BoundHealthComponent == HealthComponent)
	{
		HandleHealthChanged(HealthComponent->GetCurrentHP(), HealthComponent->GetMaxHP());
		UpdateDeathDebugUI(HealthComponent->IsDead());
		return;
	}

	UnbindHealthComponent();

	BoundHealthComponent = HealthComponent;
	BoundHealthComponent->OnHealthChanged.AddUniqueDynamic(this, &AMPPlayerController::HandleHealthChanged);
	BoundHealthComponent->OnDeath.AddUniqueDynamic(this, &AMPPlayerController::HandleDeath);

	HandleHealthChanged(BoundHealthComponent->GetCurrentHP(), BoundHealthComponent->GetMaxHP());
	UpdateDeathDebugUI(BoundHealthComponent->IsDead());
}

void AMPPlayerController::TestDamage(float DamageAmount)
{
	if (DamageAmount <= 0.0f)
	{
		DamageAmount = 10.0f;
	}

	if (HasAuthority())
	{
		ApplyDamageToControlledPawn(DamageAmount);
		return;
	}

	ServerRequestApplyTestDamage(DamageAmount);
}

void AMPPlayerController::TestHeal(float HealAmount)
{
	if (HealAmount <= 0.0f)
	{
		HealAmount = 10.0f;
	}

	if (HasAuthority())
	{
		HealControlledPawn(HealAmount);
		return;
	}

	ServerRequestHeal(HealAmount);
}

void AMPPlayerController::ServerRequestApplyTestDamage_Implementation(float DamageAmount)
{
	ApplyDamageToControlledPawn(DamageAmount);
}

void AMPPlayerController::ApplyDamageToControlledPawn(float DamageAmount)
{
	AMPActionRPGSampleCharacter* MPCharacter = Cast<AMPActionRPGSampleCharacter>(GetPawn());
	if (!MPCharacter)
	{
		return;
	}

	UMPHealthComponent* HealthComponent = MPCharacter->GetHealthComponent();
	if (!HealthComponent)
	{
		return;
	}

	HealthComponent->ApplyDamage(DamageAmount);
}

void AMPPlayerController::HandleHealthChanged(float CurrentHP, float MaxHP)
{
	UE_LOG(LogTemp, Log, TEXT("PlayerController Health Changed: %.1f / %.1f"), CurrentHP, MaxHP);

	UpdateHealthDebugUI(CurrentHP, MaxHP);
}

void AMPPlayerController::UpdateHealthDebugUI(float CurrentHP, float MaxHP)
{
	if (!NetworkDebugWidget)
	{
		return;
	}

	NetworkDebugWidget->SetHealth(CurrentHP, MaxHP);
}

void AMPPlayerController::ServerRequestHeal_Implementation(float HealAmount)
{
	HealControlledPawn(HealAmount);
}

void AMPPlayerController::HealControlledPawn(float HealAmount)
{
	AMPActionRPGSampleCharacter* MPCharacter = Cast<AMPActionRPGSampleCharacter>(GetPawn());
	if (!MPCharacter)
	{
		return;
	}

	UMPHealthComponent* HealthComponent = MPCharacter->GetHealthComponent();
	if (!HealthComponent)
	{
		return;
	}

	HealthComponent->Heal(HealAmount);
}

void AMPPlayerController::HandleDeath()
{
	UE_LOG(LogTemp, Log, TEXT("PlayerController Death Event Received"));

	UpdateDeathDebugUI(true);
}

void AMPPlayerController::UpdateDeathDebugUI(bool bDead)
{
	if (!NetworkDebugWidget)
	{
		return;
	}

	NetworkDebugWidget->SetDead(bDead);
}

void AMPPlayerController::UnbindHealthComponent()
{
	if (!BoundHealthComponent)
	{
		return;
	}

	BoundHealthComponent->OnHealthChanged.RemoveDynamic(this, &AMPPlayerController::HandleHealthChanged);
	BoundHealthComponent->OnDeath.RemoveDynamic(this, &AMPPlayerController::HandleDeath);
	BoundHealthComponent = nullptr;
}