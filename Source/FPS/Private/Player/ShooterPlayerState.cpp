// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerState.h"

AShooterPlayerState::AShooterPlayerState()
{
	NetUpdateFrequency = 100.f;
	
	ScoredKills = 0;
	Defeats = 0;
	Hits = 0;
	Misses = 0;
	bOnStreak = false;
	HeadShotKills = 0;\
	HighestStreak = 0;
	RevengeKills = 0;
	DethroneKills = 0;
	ShowStopperKills = 0;
	bFirstBlood = false;
	bWinner = false;
}

void AShooterPlayerState::AddScoredKills()
{
	++ScoredKills;
}

void AShooterPlayerState::AddDefeat()
{
	++Defeats;
}

void AShooterPlayerState::AddHit()
{
	++Hits;
}

void AShooterPlayerState::AddMiss()
{
	++Misses;
}

void AShooterPlayerState::AddHeadShotKill()
{
	++HeadShotKills;
}

void AShooterPlayerState::AddSequentialKill(int32 SequenceCount)
{
	if (SequentialKills.Contains(SequenceCount))
	{
		SequentialKills[SequenceCount]++;
	}
	else
	{
		SequentialKills.Add(SequenceCount, 1);
	}
	
	/* Reduce the count for all lower sequence counts
	 * this is because a triple kill means a double was 
	 * scored first. But we want to count this as just a triple,
	 * i.e. kill 1, kill 2, kill 3 = just a triple, not a double and a triple.
	 */
	
	for (auto& Kill : SequentialKills)
	{
		if (Kill.Key < SequenceCount && Kill.Value > 0)
		{
			Kill.Value--;
		}
	}
}

void AShooterPlayerState::UpdateHighestStreak(int32 StreakCount)
{
	if (StreakCount > HighestStreak)
	{
		HighestStreak = StreakCount;
	}
}

void AShooterPlayerState::AddRevengeKill()
{
	++RevengeKills;
}

void AShooterPlayerState::AddDethroneKill()
{
	++DethroneKills;
}

void AShooterPlayerState::AddShowStopperKill()
{
	++ShowStopperKills;
}

void AShooterPlayerState::GotFirstBlood()
{
	bFirstBlood = true;
}

void AShooterPlayerState::IsNowWinner()
{
	bWinner = true;
}

void AShooterPlayerState::SetOnStreak(bool bIsOnStreak)
{
	bOnStreak = bIsOnStreak;
}

void AShooterPlayerState::SetLastAttacker(APlayerState* Attacker)
{
	LastAttacker = Attacker;
}

APlayerState* AShooterPlayerState::GetLastAttacker() const
{
	return LastAttacker.IsValid() ? LastAttacker.Get() : nullptr;
}

bool AShooterPlayerState::IsOnStreak() const
{
	return bOnStreak;
}
