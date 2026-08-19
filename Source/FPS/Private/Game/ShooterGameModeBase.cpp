// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameModeBase.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "Game/ShooterGameStateBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerState.h"

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
		for (APlayerState* PS : ShooterGameState->PlayerArray)
		{
			if (AShooterPlayerState* ShooterPS = Cast<AShooterPlayerState>(PS))
			{
				ShooterPS->Client_MatchTimeChanged(0);
			}
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
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(MatchMap.ToString(), true);
	}
}
