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
