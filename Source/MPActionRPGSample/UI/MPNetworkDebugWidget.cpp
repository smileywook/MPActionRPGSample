#include "MPNetworkDebugWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UMPNetworkDebugWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetHealth(0.0f, 0.0f);
}

void UMPNetworkDebugWidget::SetHealth(float CurrentHP, float MaxHP)
{
    if (!HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("HP: %.0f / %.0f"), CurrentHP, MaxHP)));
    }

    if (HealthBar)
    {
        const float HealthPercent = MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f;
        HealthBar->SetPercent(HealthPercent);
    }
}