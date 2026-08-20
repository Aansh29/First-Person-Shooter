// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameOver/MatchResultWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"
#include "UI/GamePlay/ScoreboardRow.h"
#include "ShooterTypes/ShooterTypes.h"


void UMatchResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	AShooterPlayerState* PS = GetPlayerState();

	if (IsValid(PS))
	{
		PS->OnMatchResultChanged.AddUniqueDynamic(this, &ThisClass::OnMatchResultChanged);
	}
	else
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer());

		if (IsValid(PC))
		{
			PC->OnPlayerStateReplicated.AddUniqueDynamic(this, &ThisClass::OnPlayerStateReplicated);
		}
	}
}

AShooterPlayerState* UMatchResultWidget::GetPlayerState() const
{
	APlayerController* PC = GetOwningPlayer();

	if (IsValid(PC))
	{
		return PC->GetPlayerState<AShooterPlayerState>();
	}

	return nullptr;
}

void UMatchResultWidget::OnMatchResultChanged(bool bWon, const TArray<FScoreboardEntry>& Entries)
{
	if (IsValid(Text_Result))
	{
		Text_Result->SetText(bWon ? FText::FromString(TEXT("VICTORY")) : FText::FromString(TEXT("DEFEAT")));
	}

	if (IsValid(Image_Result))
	{
		Image_Result->SetColorAndOpacity(bWon ? FLinearColor::Green : FLinearColor::Red);
	}
	RefreshResultScoreboard(Entries);
	PlayMatchResultAnimation();
}

void UMatchResultWidget::OnPlayerStateReplicated()
{
	AShooterPlayerState* PS = GetPlayerState();

	if (IsValid(PS))
	{
		PS->OnMatchResultChanged.AddUniqueDynamic(this, &ThisClass::OnMatchResultChanged);
	}

	AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer());

	if (IsValid(PC))
	{
		PC->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}

void UMatchResultWidget::RefreshResultScoreboard(const TArray<FScoreboardEntry>& Entries)
{
	if (!IsValid(PlayerList) || !IsValid(ScoreboardRowClass))
	{
		return;
	}

	PlayerList->ClearChildren();

	for (const FScoreboardEntry& Entry : Entries)
	{
		UScoreboardRow* Row = CreateWidget<UScoreboardRow>(GetWorld(), ScoreboardRowClass);

		if (!IsValid(Row))
		{
			continue;
		}

		Row->SetPlayerData(Entry);

		PlayerList->AddChild(Row);
	}
}

