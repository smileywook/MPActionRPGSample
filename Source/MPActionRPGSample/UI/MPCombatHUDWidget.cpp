// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MPCombatHUDWidget.h"

#include "CommonTextBlock.h"
#include "Components/ProgressBar.h"

void UMPCombatHUDWidget::UpdateHealth(float CurrentHP, float MaxHP)
{
    const float HealthPercent = MaxHP > 0.0f ? FMath::Clamp(CurrentHP / MaxHP, 0.0f, 1.0f) : 0.0f;
    const FText CurrentHPText = FText::AsNumber(FMath::RoundToInt(CurrentHP));
    const FText MaxHPText = FText::AsNumber(FMath::RoundToInt(MaxHP));

    if (HealthText)
    {
        HealthText->SetText(FText::Format(NSLOCTEXT("CombatHUD", "HealthFormat", "HP {0} / {1}"), CurrentHPText, MaxHPText));
    }

    if (HealthBar)
    {
        HealthBar->SetPercent(HealthPercent);
    }
}