// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameStateBase.h"

#include "Player/ShooterPlayerState.h"

AShooterGameStateBase::AShooterGameStateBase()
{
	bHasFirstBloodBeenHad = false;
}

void AShooterGameStateBase::MulticastScoreInfo_Implementation(int32 CurrentMaxKill)
{
	OnScoreInfoChanged.Broadcast(CurrentMaxKill);
}

bool AShooterGameStateBase::HasFirstBloodBeenHad() const
{
	return bHasFirstBloodBeenHad;
}

void AShooterGameStateBase::UpdateLeader()
{
	TArray<APlayerState*> LocalSortedPlayers = PlayerArray;
	LocalSortedPlayers.Sort([](const APlayerState& A, const APlayerState& B)
	{
		const AShooterPlayerState* PlayerA = Cast<AShooterPlayerState>(&A);
		const AShooterPlayerState* PlayerB = Cast<AShooterPlayerState>(&B);
		return PlayerA->GetScoredKills() > PlayerB->GetScoredKills();
	});
	
	Leaders.Empty();
	
	if (LocalSortedPlayers.Num() > 0)
	{
		int32 HighestScore = 0;
		for (APlayerState* Player : LocalSortedPlayers)
		{
			AShooterPlayerState* ShooterPS = Cast<AShooterPlayerState>(Player);
			if (IsValid(ShooterPS))
			{
				int32 PlayerScore = ShooterPS->GetScoredKills();
			
				if (Leaders.Num() == 0)
				{
					HighestScore = PlayerScore;
					Leaders.Add(ShooterPS);
				}
				else if (PlayerScore == HighestScore)
				{
					Leaders.Add(ShooterPS);
				}
				else
				{
					break;
				}
			}
		}
	}
	
	bHasFirstBloodBeenHad = true;
}

AShooterPlayerState* AShooterGameStateBase::GetSoleLeader() const
{
	if (Leaders.Num() == 1)
	{
		return Leaders[0];
	}
	return nullptr;
}

AShooterPlayerState* AShooterGameStateBase::GetTopScorer() const
{
	if (Leaders.Num() > 0)
	{
		return Leaders[0];
	}

	return nullptr;
}

int32 AShooterGameStateBase::GetTopScore() const
{
	if (AShooterPlayerState* TopScorer = GetTopScorer())
	{
		return TopScorer->GetScoredKills();
	}
	return 0;
}

bool AShooterGameStateBase::IsTiedForTheLead(AShooterPlayerState* PlayerState)
{
	return Leaders.Contains(PlayerState) && Leaders.Num() > 1;
}
