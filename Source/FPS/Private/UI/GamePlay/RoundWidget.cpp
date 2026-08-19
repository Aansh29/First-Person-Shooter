#include "UI/GamePlay/RoundWidget.h"

#include "Components/TextBlock.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"
#include "Game/ShooterGameStateBase.h"
#include "TimerManager.h"

void URoundWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	AShooterPlayerState* PS = GetPlayerState();

	if (IsValid(PS))
	{
		PS->OnMatchTimeChanged.AddUniqueDynamic(
			this,
			&ThisClass::SetMatchTime
		);
		
		AShooterGameStateBase* GameState = GetWorld()->GetGameState<AShooterGameStateBase>();

		if (IsValid(GameState))
		{
			GameState->OnScoreInfoChanged.AddUniqueDynamic(this, &ThisClass::SetScoreInfo);
		}
		SetMatchTime(PS->GetCurrentMatchTime());
	}
	else
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer());

		if (IsValid(PC))
		{
			PC->OnPlayerStateReplicated.AddUniqueDynamic(
				this,
				&ThisClass::OnPlayerStateReplicated
			);
		}
	}
}

void URoundWidget::OnPlayerStateReplicated()
{
	AShooterPlayerState* PS = GetPlayerState();

	AShooterGameStateBase* GameState = GetWorld()->GetGameState<AShooterGameStateBase>();

	if (IsValid(GameState))
	{
		GameState->OnScoreInfoChanged.AddUniqueDynamic(this, &ThisClass::SetScoreInfo);
	}
	
	if (IsValid(PS))
	{
		PS->OnMatchTimeChanged.AddUniqueDynamic(
			this,
			&ThisClass::SetMatchTime
		);
		
		SetMatchTime(PS->GetCurrentMatchTime());
	}

	if (AShooterPlayerController* PC =Cast<AShooterPlayerController>(GetOwningPlayer());IsValid(PC))
	{
		PC->OnPlayerStateReplicated.RemoveDynamic(
			this,
			&ThisClass::OnPlayerStateReplicated
		);
	}
}

AShooterPlayerState* URoundWidget::GetPlayerState() const
{
	APlayerController* PC = GetOwningPlayer();

	if (IsValid(PC))
	{
		return PC->GetPlayerState<AShooterPlayerState>();
	}

	return nullptr;
}

void URoundWidget::SetMatchTime(int32 RemainingTime)
{
	CurrentMatchTime = RemainingTime;

	UpdateLocalTimer();
	
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(LocalTimerHandle);

	if (CurrentMatchTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			LocalTimerHandle,
			this,
			&ThisClass::UpdateLocalTimer,
			1.0f,
			true
		);
	}
}

void URoundWidget::UpdateLocalTimer()
{
	if (CurrentMatchTime <= 0)
	{
		CurrentMatchTime = 0;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(LocalTimerHandle);
		}
	}
	else
	{
		--CurrentMatchTime;
	}

	if (IsValid(Text_Round))
	{
		const int32 Minutes = CurrentMatchTime / 60;
		const int32 Seconds = CurrentMatchTime % 60;

		Text_Round->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("%02d:%02d"),
					Minutes,
					Seconds
				)
			)
		);
	}
}

void URoundWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(LocalTimerHandle);
	}

	if (AShooterPlayerState* PS = GetPlayerState())
	{
		PS->OnMatchTimeChanged.RemoveDynamic(
			this,
			&ThisClass::SetMatchTime
		);
	}

	if (AShooterGameStateBase* GameState = GetWorld()->GetGameState<AShooterGameStateBase>(); IsValid(GameState))
	{
		GameState->OnScoreInfoChanged.RemoveDynamic(this, &ThisClass::SetScoreInfo);
	}

	Super::NativeDestruct();
}

void URoundWidget::SetScoreInfo(int32 CurrentMaxKill)
{
	if (IsValid(Text_CurrentMaxKill))
	{
		Text_CurrentMaxKill->SetText(FText::AsNumber(CurrentMaxKill));
	}
}