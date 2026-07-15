// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MPSkillComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPACTIONRPGSAMPLE_API UMPSkillComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	bool bDebugSkillLog = true;

public:	
	// Sets default values for this component's properties
	UMPSkillComponent();

	bool HasSkillAuthority() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void PrintSkillLog(const FString& Message) const;
};
