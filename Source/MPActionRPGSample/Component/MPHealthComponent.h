#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MPHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRespawn);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPACTIONRPGSAMPLE_API UMPHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnRespawn OnRespawn;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float CurrentHP = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float MaxHP = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "Health")
    bool bIsDead = false;

public:
    UMPHealthComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetCurrentHP() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetMaxHP() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDead() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ResetForRespawn();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnRep_CurrentHP();

    UFUNCTION()
    void OnRep_IsDead();

    void SetCurrentHP(float NewHP);

    void HandleDeath();
};