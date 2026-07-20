// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MPMonsterCharacter.generated.h"

class UMPHealthComponent;

UCLASS()
class MPACTIONRPGSAMPLE_API AMPMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMPHealthComponent> HealthComponent;

public:
	// Sets default values for this character's properties
	AMPMonsterCharacter();

	UMPHealthComponent* GetHealthComponent() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
