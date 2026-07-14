// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MPNetworkDebugWidget.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API UMPNetworkDebugWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HealthText;
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> HealthBar;

public:
    void SetHealth(float CurrentHP, float MaxHP);

protected:
    virtual void NativeConstruct() override;
	
};
