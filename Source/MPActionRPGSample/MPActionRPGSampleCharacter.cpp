// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPActionRPGSampleCharacter.h"
#include "MPPlayerState.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Component/MPHealthComponent.h"
#include "Component/MPSkillComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "TimerManager.h"

namespace
{
	FString NetModeToString(ENetMode NetMode)
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

	FString RoleToString(ENetRole Role)
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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMPActionRPGSampleCharacter

AMPActionRPGSampleCharacter::AMPActionRPGSampleCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	bReplicates = true;
	HealthComponent = CreateDefaultSubobject<UMPHealthComponent>(TEXT("HealthComponent"));
	SkillComponent = CreateDefaultSubobject<UMPSkillComponent>(TEXT("SkillComponent"));
}

void AMPActionRPGSampleCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[Character BeginPlay] Name=%s | NetMode=%s | LocalRole=%s | RemoteRole=%s | HasAuthority=%s | IsLocallyControlled=%s"),
		*GetName(),
		*NetModeToString(GetNetMode()),
		*RoleToString(GetLocalRole()),
		*RoleToString(GetRemoteRole()),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		IsLocallyControlled() ? TEXT("true") : TEXT("false"));
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMPActionRPGSampleCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMPActionRPGSampleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMPActionRPGSampleCharacter::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMPActionRPGSampleCharacter::StopJump);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMPActionRPGSampleCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMPActionRPGSampleCharacter::Look);

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMPActionRPGSampleCharacter::Move(const FInputActionValue& Value)
{
	if (IsCharacterDead())
	{
		return;
	}

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMPActionRPGSampleCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMPActionRPGSampleCharacter::StartJump()
{
	if (IsCharacterDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Input][JumpRejected] Character=%s Reason=Dead"), *GetName());
		return;
	}

	Jump();
}

void AMPActionRPGSampleCharacter::StopJump()
{
	StopJumping();
}

void AMPActionRPGSampleCharacter::Attack()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (IsCharacterDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack][InputRejected] Character=%s Reason=Dead"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Attack][Input] Character=%s HasAuthority=%d LocalRole=%d RemoteRole=%d"),
		*GetName(), HasAuthority(), static_cast<int32>(GetLocalRole()), static_cast<int32>(GetRemoteRole()));

	ServerStartAttack();
}

void AMPActionRPGSampleCharacter::ServerStartAttack_Implementation()
{
	FString FailReason;
	if (!CanAttack(FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack][Rejected] Character=%s Reason=%s"), *GetName(), *FailReason);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Attack][Accepted] Character=%s HasAuthority=%d"), *GetName(), HasAuthority());

	HandleAttack();
}

bool AMPActionRPGSampleCharacter::CanAttack(FString& OutFailReason) const
{
	OutFailReason = TEXT("None");

	if (!HasAuthority())
	{
		OutFailReason = TEXT("NoAuthority");
		return false;
	}

	if (!HealthComponent)
	{
		OutFailReason = TEXT("NoHealthComponent");
		return false;
	}

	if (HealthComponent->IsDead())
	{
		OutFailReason = TEXT("Dead");
		return false;
	}

	if (bIsAttacking)
	{
		OutFailReason = TEXT("AlreadyAttacking");
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		OutFailReason = TEXT("NoWorld");
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	const float ElapsedTime = CurrentTime - LastAttackTime;

	if (ElapsedTime < AttackCooldown)
	{
		OutFailReason = FString::Printf(TEXT("Cooldown %.2f / %.2f"), ElapsedTime, AttackCooldown);
		return false;
	}

	return true;
}

void AMPActionRPGSampleCharacter::HandleAttack()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsAttacking = true;

	if (UWorld* World = GetWorld())
	{
		LastAttackTime = World->GetTimeSeconds();
	}

	GetWorldTimerManager().ClearTimer(AttackFinishTimerHandle);
	GetWorldTimerManager().SetTimer(AttackFinishTimerHandle, this, &ThisClass::FinishAttack, AttackDuration, false);

	APlayerState* PS = GetPlayerState();
	const FString PlayerName = PS ? PS->GetPlayerName() : TEXT("Unknown");

	UE_LOG(LogTemp, Warning, TEXT("[Attack][Started] Player=%s Character=%s Cooldown=%.2f Duration=%.2f"), *PlayerName, *GetName(), AttackCooldown, AttackDuration);	

	FHitResult HitResult;
	if (PerformAttackTrace(HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		const FString HitActorName = HitActor ? HitActor->GetName() : TEXT("None");

		UE_LOG(LogTemp, Warning, TEXT("[Attack][TraceHit] Attacker=%s HitActor=%s HitLocation=%s"), *GetName(), *HitActorName, *HitResult.ImpactPoint.ToString());

		ApplyAttackDamageToActor(HitActor);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[Attack][TraceMiss] Attacker=%s Range=%.2f Radius=%.2f"), *GetName(), AttackRange, AttackTraceRadius);
	}
}

void AMPActionRPGSampleCharacter::FinishAttack()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsAttacking = false;

	UE_LOG(LogTemp, Log, TEXT("[Attack][Finished] Character=%s"), *GetName());
}

bool AMPActionRPGSampleCharacter::PerformAttackTrace(FHitResult& OutHit) const
{
	if (!HasAuthority())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, BaseEyeHeight);
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackTrace), false, this);
	QueryParams.AddIgnoredActor(this);

	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(AttackTraceRadius);
	const bool bHit = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, ECC_Pawn, CollisionShape, QueryParams);

	if (bDrawAttackTrace)
	{
		const FColor DrawColor = bHit ? FColor::Red : FColor::Green;

		DrawDebugLine(World, Start, End, DrawColor, false, 1.0f, 0, 2.0f);
		DrawDebugSphere(World, Start, AttackTraceRadius, 12, DrawColor, false, 1.0f);
		DrawDebugSphere(World, End, AttackTraceRadius, 12, DrawColor, false, 1.0f);

		if (bHit)
		{
			DrawDebugSphere(World, OutHit.ImpactPoint, AttackTraceRadius, 12, FColor::Yellow, false, 1.0f);
		}
	}

	return bHit && OutHit.GetActor();
}

bool AMPActionRPGSampleCharacter::ApplyAttackDamageToActor(AActor* TargetActor)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack][DamageFailed] Attacker=%s Reason=NoTargetActor"), *GetName());
		return false;
	}

	if (TargetActor == this)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack][DamageFailed] Attacker=%s Reason=SelfTarget"), *GetName());
		return false;
	}

	UMPHealthComponent* TargetHealthComponent = TargetActor->FindComponentByClass<UMPHealthComponent>();
	if (!TargetHealthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack][DamageFailed] Attacker=%s Target=%s Reason=NoHealthComponent"), *GetName(), *TargetActor->GetName());
		return false;
	}

	if (TargetHealthComponent->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack][DamageFailed] Attacker=%s Target=%s Reason=TargetDead"), *GetName(), *TargetActor->GetName());
		return false;
	}

	TargetHealthComponent->ApplyDamage(AttackDamage);

	UE_LOG(LogTemp, Warning, TEXT("[Attack][DamageApplied] Attacker=%s Target=%s Damage=%.2f"), *GetName(), *TargetActor->GetName(), AttackDamage);
	return true;
}

bool AMPActionRPGSampleCharacter::IsCharacterDead() const
{
	return HealthComponent && HealthComponent->IsDead();
}

UMPHealthComponent* AMPActionRPGSampleCharacter::GetHealthComponent() const
{
	return HealthComponent;
}

UMPSkillComponent* AMPActionRPGSampleCharacter::GetSkillComponent() const
{
	return SkillComponent;
}
