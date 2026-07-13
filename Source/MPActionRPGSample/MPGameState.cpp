// Fill out your copyright notice in the Description page of Project Settings.


#include "MPGameState.h"
#include "MPPlayerState.h"

FText AMPGameState::GetPlayerListText() const
{
	FString Result = TEXT("Players:\n");

	if (PlayerArray.Num() == 0)
	{
		Result += TEXT("  Empty");
		return FText::FromString(Result);
	}

	for (int32 Index = 0; Index < PlayerArray.Num(); ++Index)
	{
		const APlayerState* BasePlayerState = PlayerArray[Index];

		if (!BasePlayerState)
		{
			Result += FString::Printf(TEXT("  [%d] Invalid PlayerState\n"), Index);
			continue;
		}

		const AMPPlayerState* MPPlayerState = Cast<AMPPlayerState>(BasePlayerState);
		if (MPPlayerState)
		{
			Result += FString::Printf(TEXT("  [%d] %s\n"), Index, *MPPlayerState->GetPlayerDisplayName());
		}
		else
		{
			Result += FString::Printf(TEXT("  [%d] %s\n"), Index, *BasePlayerState->GetPlayerName());
		}
	}

	return FText::FromString(Result);
}

int32 AMPGameState::GetConnectedPlayerCount() const
{
	return PlayerArray.Num();
}
