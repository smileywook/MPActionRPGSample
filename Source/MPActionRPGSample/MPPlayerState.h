// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API AMPPlayerState : public APlayerState
{
	GENERATED_BODY()

private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerDisplayName)
	FString PlayerDisplayName;

public:
	AMPPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "MP|PlayerState")
	const FString& GetPlayerDisplayName() const { return PlayerDisplayName; }

	void SetPlayerDisplayName(const FString& NewDisplayName);

protected:
	UFUNCTION()
	void OnRep_PlayerDisplayName();

private:
	void HandlePlayerDisplayNameChanged();
	
};
