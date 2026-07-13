// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MPGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API AMPGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AMPGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
};
