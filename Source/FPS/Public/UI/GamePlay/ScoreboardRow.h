// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardRow.generated.h"

struct FScoreboardEntry;
class UTextBlock;
/**
 * 
 */
UCLASS()
class FPS_API UScoreboardRow : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetPlayerData(const FScoreboardEntry& Entry);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Kills;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Deaths;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Ping;
};
