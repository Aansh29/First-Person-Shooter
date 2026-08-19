// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GamePlay/ScoreboardRow.h"
#include "ShooterTypes/ShooterTypes.h"
#include "Components/TextBlock.h"


void UScoreboardRow::SetPlayerData(const FScoreboardEntry& Entry)
{
	if (IsValid(PlayerName))
	{
		PlayerName->SetText(FText::FromString(Entry.PlayerName));
	}

	if (IsValid(Kills))
	{
		Kills->SetText(FText::AsNumber(Entry.Kills));
	}

	if (IsValid(Deaths))
	{
		Deaths->SetText(FText::AsNumber(Entry.Deaths));
	}

	if (IsValid(Ping))
	{
		Ping->SetText(FText::AsNumber(Entry.Ping));
	}
}
