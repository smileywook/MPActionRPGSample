#include "MPHealthComponent.h"
#include "Net/UnrealNetwork.h"

UMPHealthComponent::UMPHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UMPHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        CurrentHP = MaxHP;
    }

    OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void UMPHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMPHealthComponent, CurrentHP);
    DOREPLIFETIME(UMPHealthComponent, bIsDead);
}

float UMPHealthComponent::GetCurrentHP() const
{
    return CurrentHP;
}

float UMPHealthComponent::GetMaxHP() const
{
    return MaxHP;
}

bool UMPHealthComponent::IsDead() const
{
    return bIsDead;
}

void UMPHealthComponent::ApplyDamage(float DamageAmount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    if (bIsDead)
    {
        return;
    }

    if (DamageAmount <= 0.0f)
    {
        return;
    }

    SetCurrentHP(CurrentHP - DamageAmount);
}

void UMPHealthComponent::Heal(float HealAmount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    if (bIsDead)
    {
        return;
    }

    if (HealAmount <= 0.0f)
    {
        return;
    }

    SetCurrentHP(CurrentHP + HealAmount);
}

void UMPHealthComponent::SetCurrentHP(float NewHP)
{
    const float ClampedHP = FMath::Clamp(NewHP, 0.0f, MaxHP);

    if (FMath::IsNearlyEqual(CurrentHP, ClampedHP))
    {
        if (CurrentHP <= 0.0f && !bIsDead)
        {
            HandleDeath();
        }

        return;
    }

    CurrentHP = ClampedHP;
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);

    if (CurrentHP <= 0.0f && !bIsDead)
    {
        HandleDeath();
    }
}

void UMPHealthComponent::HandleDeath()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    OnDeath.Broadcast();
}

void UMPHealthComponent::OnRep_CurrentHP()
{
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void UMPHealthComponent::OnRep_IsDead()
{
    if (bIsDead)
    {
        OnDeath.Broadcast();
    }
}
