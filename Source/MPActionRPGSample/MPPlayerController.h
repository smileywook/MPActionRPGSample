// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> NetworkDebugWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NetworkDebugWidget;
	
protected:
	virtual void BeginPlay() override;
};
