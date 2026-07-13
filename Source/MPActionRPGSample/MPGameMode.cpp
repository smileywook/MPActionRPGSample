// Fill out your copyright notice in the Description page of Project Settings.


#include "MPGameMode.h"

#include "MPGameState.h"
#include "MPPlayerController.h"
#include "MPPlayerState.h"
#include "MPActionRPGSampleCharacter.h"

AMPGameMode::AMPGameMode()
{
	GameStateClass = AMPGameState::StaticClass();
	PlayerControllerClass = AMPPlayerController::StaticClass();
	PlayerStateClass = AMPPlayerState::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		//DefaultPawnClass = AMPActionRPGSampleCharacter::StaticClass();
	}
}

void AMPGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	AMPPlayerState* MPPlayerState = NewPlayer->GetPlayerState<AMPPlayerState>();
	if (!MPPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] PostLogin failed. PlayerState is invalid. Controller=%s"), *NewPlayer->GetName());

		return;
	}

	const int32 PlayerId = MPPlayerState->GetPlayerId();
	const FString NewDisplayName = FString::Printf(TEXT("Player_%d"), PlayerId);

	MPPlayerState->SetPlayerDisplayName(NewDisplayName);

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] PostLogin. Controller=%s | PlayerState=%s | DisplayName=%s"),
		*NewPlayer->GetName(),
		*MPPlayerState->GetName(),
		*NewDisplayName);
}
