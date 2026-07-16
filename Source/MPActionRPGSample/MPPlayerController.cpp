// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"
#include "MPActionRPGSampleCharacter.h"
#include "MPPlayerState.h"
#include "Component/MPHealthComponent.h"
#include "Component/MPSkillComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/MPNetworkDebugWidget.h"
#include "TimerManager.h"

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

			SkillCooldownText = Cast<UTextBlock>(NetworkDebugWidget->GetWidgetFromName(TEXT("SkillCooldownText")));
			SkillCooldownBar = Cast<UProgressBar>(NetworkDebugWidget->GetWidgetFromName(TEXT("SkillCooldownBar")));
		}
	}

	TryBindPlayerStateEvents();
}

void AMPPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHealthComponent();
	UnbindSkillComponent();

	Super::EndPlay(EndPlayReason);
}

void AMPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	TryBindHealthComponent();
	TryBindSkillComponent();
}

void AMPPlayerController::OnUnPossess()
{
	UnbindHealthComponent();
	UnbindSkillComponent();

	Super::OnUnPossess();
}

void AMPPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서는 BeginPlay 시점에 PlayerState가 아직 준비되지 않을 수 있으므로
	// PlayerState 복제 완료 시점에도 바인딩을 다시 시도한다.
	TryBindPlayerStateEvents();

	TryBindHealthComponent();
	TryBindSkillComponent();
}

void AMPPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	TryBindHealthComponent();
	TryBindSkillComponent();
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
	BoundHealthComponent->OnRespawn.AddUniqueDynamic(this, &AMPPlayerController::HandleRespawn);

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

void AMPPlayerController::TestRespawn()
{
	UE_LOG(LogTemp, Log, TEXT("[Respawn][Input] Controller=%s HasAuthority=%d"), *GetName(), HasAuthority());

	if (HasAuthority())
	{
		RequestRespawnControlledPawn();
		return;
	}

	ServerRequestRespawn();
}

void AMPPlayerController::ServerRequestRespawn_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[Respawn][ServerRequest] Controller=%s"), *GetName());

	RequestRespawnControlledPawn();
}

void AMPPlayerController::RequestRespawnControlledPawn()
{
	if (!HasAuthority())
	{
		return;
	}

	AMPActionRPGSampleCharacter* MPCharacter = Cast<AMPActionRPGSampleCharacter>(GetPawn());
	if (!MPCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Respawn][Rejected] Controller=%s Reason=NoPawn"), *GetName());
		return;
	}

	UMPHealthComponent* HealthComponent = MPCharacter->GetHealthComponent();
	if (!HealthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Respawn][Rejected] Controller=%s Pawn=%s Reason=NoHealthComponent"), *GetName(), *GetNameSafe(MPCharacter));
		return;
	}

	if (!HealthComponent->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Respawn][Rejected] Controller=%s Pawn=%s Reason=NotDead"), *GetName(), *GetNameSafe(MPCharacter));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Respawn][Accepted] Controller=%s Pawn=%s"), *GetName(), *GetNameSafe(MPCharacter));

	RespawnControlledPawn(MPCharacter, HealthComponent);
}

void AMPPlayerController::RespawnControlledPawn(AMPActionRPGSampleCharacter* MPCharacter, UMPHealthComponent* HealthComponent)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!MPCharacter || !HealthComponent)
	{
		return;
	}

	if (!MovePawnToRespawnLocation(MPCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Respawn][Failed] Controller=%s Pawn=%s Reason=MoveFailed"), *GetName(), *GetNameSafe(MPCharacter));
		return;
	}

	HealthComponent->ResetForRespawn();

	UE_LOG(LogTemp, Warning, TEXT("[Respawn][Completed] Controller=%s Pawn=%s HP=%.1f/%.1f"), *GetName(), *GetNameSafe(MPCharacter), HealthComponent->GetCurrentHP(), HealthComponent->GetMaxHP());
}

bool AMPPlayerController::MovePawnToRespawnLocation(AMPActionRPGSampleCharacter* MPCharacter)
{
	if (!HasAuthority() || !MPCharacter)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	AGameModeBase* GameMode = World->GetAuthGameMode();
	AActor* PlayerStartSpot = GameMode ? GameMode->FindPlayerStart(this) : nullptr;
	if (!PlayerStartSpot)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Respawn][MoveFailed] Controller=%s Reason=NoPlayerStart"), *GetName());
		return false;
	}

	const FVector RespawnLocation = PlayerStartSpot->GetActorLocation();
	const FRotator RespawnRotation = PlayerStartSpot->GetActorRotation();

	if (UCharacterMovementComponent* MovementComponent = MPCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	MPCharacter->SetActorLocationAndRotation(RespawnLocation, RespawnRotation, false, nullptr, ETeleportType::TeleportPhysics);
	SetControlRotation(RespawnRotation);

	UE_LOG(LogTemp, Warning, TEXT("[Respawn][Moved] Controller=%s Pawn=%s Location=%s Rotation=%s"), *GetName(), *GetNameSafe(MPCharacter), *RespawnLocation.ToString(), *RespawnRotation.ToString());

	return true;
}

void AMPPlayerController::HandleDeath()
{
	UE_LOG(LogTemp, Log, TEXT("PlayerController Death Event Received"));

	UpdateDeathDebugUI(true);
}

void AMPPlayerController::HandleRespawn()
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Respawn Event Received"));

	if (BoundHealthComponent)
	{
		HandleHealthChanged(BoundHealthComponent->GetCurrentHP(), BoundHealthComponent->GetMaxHP());
	}

	UpdateDeathDebugUI(false);
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
	BoundHealthComponent->OnRespawn.RemoveDynamic(this, &AMPPlayerController::HandleRespawn);
	BoundHealthComponent = nullptr;
}

void AMPPlayerController::TryBindSkillComponent()
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

	UMPSkillComponent* SkillComponent = MPCharacter->GetSkillComponent();
	if (!SkillComponent)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SkillUI] BindSkillComponent Controller=%s Component=%s"), *GetNameSafe(this), *GetNameSafe(SkillComponent));


	if (BoundSkillComponent == SkillComponent)
	{
		HandleSkillCooldownChanged();
		return;
	}

	UnbindSkillComponent();

	BoundSkillComponent = SkillComponent;
	SkillComponent->OnSkillCooldownChanged.AddDynamic(this, &AMPPlayerController::HandleSkillCooldownChanged);

	HandleSkillCooldownChanged();
}

void AMPPlayerController::HandleSkillCooldownChanged()
{
	if (!IsLocalController())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SkillUI] HandleSkillCooldownChanged Controller=%s Component=%s"), *GetNameSafe(this), *GetNameSafe(BoundSkillComponent.Get()));

	GetWorldTimerManager().ClearTimer(SkillCooldownUITimerHandle);
	UpdateSkillCooldownUI();

	const UMPSkillComponent* SkillComponent = BoundSkillComponent.Get();
	if (!SkillComponent || !SkillComponent->IsSkillOnCooldown())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(SkillCooldownUITimerHandle, this, &AMPPlayerController::UpdateSkillCooldownUI, 0.1f, true);
}

void AMPPlayerController::UpdateSkillCooldownUI()
{
	UMPSkillComponent* SkillComponent = BoundSkillComponent.Get();
	if (!SkillComponent)
	{
		GetWorldTimerManager().ClearTimer(SkillCooldownUITimerHandle);
		return;
	}

	const float RemainingCooldown = SkillComponent->GetRemainingCooldown();
	const float CooldownDuration = SkillComponent->GetCooldownDuration();
	const FString DisplayName = SkillComponent->GetSkillData().DisplayName;
	const bool bIsOnCooldown = RemainingCooldown > KINDA_SMALL_NUMBER;

	if (SkillCooldownText.IsValid())
	{
		const FString CooldownText = bIsOnCooldown
			? FString::Printf(TEXT("%s: %.1fs"), *DisplayName, RemainingCooldown)
			: FString::Printf(TEXT("%s: Ready"), *DisplayName);

		SkillCooldownText->SetText(FText::FromString(CooldownText));
	}

	if (SkillCooldownBar.IsValid())
	{
		const float CooldownPercent = CooldownDuration > KINDA_SMALL_NUMBER
			? 1.0f - FMath::Clamp(RemainingCooldown / CooldownDuration, 0.0f, 1.0f)
			: 1.0f;

		SkillCooldownBar->SetPercent(CooldownPercent);
	}

	if (!bIsOnCooldown)
	{
		GetWorldTimerManager().ClearTimer(SkillCooldownUITimerHandle);
	}
}

void AMPPlayerController::UnbindSkillComponent()
{
	if (UMPSkillComponent* SkillComponent = BoundSkillComponent.Get())
	{
		SkillComponent->OnSkillCooldownChanged.RemoveDynamic(this, &AMPPlayerController::HandleSkillCooldownChanged);
	}

	GetWorldTimerManager().ClearTimer(SkillCooldownUITimerHandle);
	BoundSkillComponent.Reset();

	if (SkillCooldownText.IsValid())
	{
		SkillCooldownText->SetText(FText::FromString(TEXT("Ground Slash: Ready")));
	}

	if (SkillCooldownBar.IsValid())
	{
		SkillCooldownBar->SetPercent(1.0f);
	}
}

