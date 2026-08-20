// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameModeBase.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "Game/ShooterGameStateBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerState.h"
#include "ShooterTypes/ShooterTypes.h"

AShooterGameModeBase::AShooterGameModeBase()
{
	MatchDuration = 120.f;
	ResultsDisplayTime = 10.f;
	TargetKills = 20;
}

void AShooterGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	bMatchFinished = false;
	StartMatchTimer();
}

void AShooterGameModeBase::RequestRespawn(ACharacter* Character, AController* Controller)
{
	if (!IsValid(Character) || !IsValid(Controller)) return;
	
	Character->Reset();
	Character->Destroy();
	
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
	ensure(PlayerStarts.Num() > 0);
	int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
	
	RestartPlayerAtPlayerStart(Controller, PlayerStarts[Selection]);
}

void AShooterGameModeBase::StartMatchTimer()
{
	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&AShooterGameModeBase::FinishMatch,
		MatchDuration,
		false
	);
	
	BroadcastMatchTime();
	
	GetWorldTimerManager().SetTimer(
		MatchTimeBroadcastTimerHandle,
		this,
		&AShooterGameModeBase::BroadcastMatchTime,
		5.f,
		true
	);
}

void AShooterGameModeBase::BroadcastMatchTime()
{
	const float RemainingTime = GetRemainingMatchTime();

	const int32 RemainingSeconds = FMath::Max(
		0,
		FMath::CeilToInt(RemainingTime)
	);

	AShooterGameStateBase* ShooterGameState = GetGameState<AShooterGameStateBase>();

	if (!IsValid(ShooterGameState))
	{
		return;
	}

	for (APlayerState* PS : ShooterGameState->PlayerArray)
	{
		if (AShooterPlayerState* ShooterPS = Cast<AShooterPlayerState>(PS))
		{
			ShooterPS->Client_MatchTimeChanged(RemainingSeconds);
		}
	}
}

float AShooterGameModeBase::GetRemainingMatchTime() const
{
	if (!GetWorld())
	{
		return 0.f;
	}

	return GetWorldTimerManager().GetTimerRemaining(MatchTimerHandle);
}

void AShooterGameModeBase::FinishMatch()
{
	if (bMatchFinished)
	{
		return;
	}

	bMatchFinished = true;

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	GetWorldTimerManager().ClearTimer(MatchTimeBroadcastTimerHandle);

	AShooterGameStateBase* ShooterGameState = GetGameState<AShooterGameStateBase>();

	if (IsValid(ShooterGameState))
	{
		TArray<FScoreboardEntry> Entries;

		for (APlayerState* PS : ShooterGameState->PlayerArray)
		{
			AShooterPlayerState* ShooterPS = Cast<AShooterPlayerState>(PS);

			if (!IsValid(ShooterPS))
			{
				continue;
			}

			FScoreboardEntry Entry;

			Entry.PlayerName = ShooterPS->GetPlayerName();
			Entry.Kills = ShooterPS->GetScoredKills();
			Entry.Deaths = ShooterPS->GetDefeats();
			Entry.RevengeKills = ShooterPS->GetRevengeKills();
			Entry.Ping = ShooterPS->GetCompressedPing() * 4;

			Entries.Add(Entry);
		}

		// Ranking:
		// 1. Highest kills
		// 2. Highest revenge kills
		// 3. Lowest deaths
		Entries.Sort([](const FScoreboardEntry& A, const FScoreboardEntry& B)
		{
			if (A.Kills != B.Kills)
			{
				return A.Kills > B.Kills;
			}

			if (A.RevengeKills != B.RevengeKills)
			{
				return A.RevengeKills > B.RevengeKills;
			}

			return A.Deaths < B.Deaths;
		});

		const int32 HighestKills = Entries.Num() > 0 ? Entries[0].Kills : 0;

		// Match is over for everyone.
		// Send each player their own win/lose result
		// together with the complete final scoreboard.
		for (APlayerState* PS : ShooterGameState->PlayerArray)
		{
			AShooterPlayerState* ShooterPS = Cast<AShooterPlayerState>(PS);

			if (!IsValid(ShooterPS))
			{
				continue;
			}

			const bool bWon = ShooterPS->GetScoredKills() == HighestKills;

			ShooterPS->Client_MatchTimeChanged(0);
			ShooterPS->Client_MatchResult(bWon, Entries);
		}
	}

	GetWorldTimerManager().SetTimer(
		ResultsTimerHandle,
		this,
		&AShooterGameModeBase::RestartMatch,
		ResultsDisplayTime,
		false
	);
}

void AShooterGameModeBase::RestartMatch()
{
	UWorld* World = GetWorld();
	
	if (IsValid(World))
	{
		World->ServerTravel(MatchMap.ToString());
	}
}
