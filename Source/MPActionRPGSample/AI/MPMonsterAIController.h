// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MPMonsterAIController.generated.h"

/**
 * 
 */
UCLASS()
class MPACTIONRPGSAMPLE_API AMPMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMPMonsterAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	
};
