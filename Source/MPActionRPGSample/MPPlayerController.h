// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

class AMPPlayerState;
class UUserWidget;

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

private:
	UPROPERTY()
	TObjectPtr<AMPPlayerState> CachedMPPlayerState;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

private:
	void TryBindPlayerStateEvents();

	UFUNCTION()
	void HandlePlayerDisplayNameChanged(const FString& NewDisplayName);
};
