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
        UE_LOG(LogTemp, Warning, TEXT("[Health][DamageIgnored] Owner=%s Reason=Dead Damage=%.2f"), *GetNameSafe(GetOwner()), DamageAmount);
        return;
    }

    if (DamageAmount <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Health][DamageIgnored] Owner=%s Reason=InvalidAmount Damage=%.2f"), *GetNameSafe(GetOwner()), DamageAmount);
        return;
    }

    const float OldHP = CurrentHP;
    SetCurrentHP(CurrentHP - DamageAmount);

    UE_LOG(LogTemp, Warning, TEXT("[Health][DamageApplied] Owner=%s Damage=%.2f HP=%.1f->%.1f"), *GetNameSafe(GetOwner()), DamageAmount, OldHP, CurrentHP);
}

void UMPHealthComponent::Heal(float HealAmount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    if (bIsDead)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Health][HealIgnored] Owner=%s Reason=Dead Heal=%.2f"), *GetNameSafe(GetOwner()), HealAmount);
        return;
    }

    if (HealAmount <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Health][HealIgnored] Owner=%s Reason=InvalidAmount Heal=%.2f"), *GetNameSafe(GetOwner()), HealAmount);
        return;
    }

    const float OldHP = CurrentHP;
    SetCurrentHP(CurrentHP + HealAmount);

    UE_LOG(LogTemp, Log, TEXT("[Health][HealApplied] Owner=%s Heal=%.2f HP=%.1f->%.1f"), *GetNameSafe(GetOwner()), HealAmount, OldHP, CurrentHP);
}

void UMPHealthComponent::SetCurrentHP(float NewHP)
{
    const float OldHP = CurrentHP;
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

    UE_LOG(LogTemp, Log, TEXT("[Health][Changed] Owner=%s HP=%.1f->%.1f MaxHP=%.1f"), *GetNameSafe(GetOwner()), OldHP, CurrentHP, MaxHP);

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

    UE_LOG(LogTemp, Warning, TEXT("[Health][DeathConfirmed] Owner=%s HP=%.1f"), *GetNameSafe(GetOwner()), CurrentHP);
}

void UMPHealthComponent::OnRep_CurrentHP()
{
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);

    UE_LOG(LogTemp, Log, TEXT("[Health][OnRep_CurrentHP] Owner=%s HP=%.1f MaxHP=%.1f"), *GetNameSafe(GetOwner()), CurrentHP, MaxHP);
}

void UMPHealthComponent::OnRep_IsDead()
{
    UE_LOG(LogTemp, Warning, TEXT("[Health][OnRep_IsDead] Owner=%s bIsDead=%s"), *GetNameSafe(GetOwner()), bIsDead ? TEXT("true") : TEXT("false"));

    if (bIsDead)
    {
        OnDeath.Broadcast();
    }
}
