// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPActionRPGSampleGameMode.h"
#include "MPActionRPGSampleCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMPActionRPGSampleGameMode::AMPActionRPGSampleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
