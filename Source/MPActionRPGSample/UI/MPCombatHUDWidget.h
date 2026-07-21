// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MPCombatHUDWidget.generated.h"

class UCommonTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API UMPCombatHUDWidget : public UCommonUserWidget
{
	GENERATED_BODY()

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> HealthText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthBar;

public:
    void UpdateHealth(float CurrentHP, float MaxHP);
	
};
