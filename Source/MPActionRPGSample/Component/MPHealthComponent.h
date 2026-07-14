#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MPHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHP, float, MaxHP);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPACTIONRPGSAMPLE_API UMPHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChanged OnHealthChanged;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float CurrentHP = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float MaxHP = 100.0f;

public:
    UMPHealthComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetCurrentHP() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetMaxHP() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnRep_CurrentHP();

    void SetCurrentHP(float NewHP);
};