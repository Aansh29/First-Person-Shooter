// Fill out your copyright notice in the Description page of Project Settings.


#include "Elimination/EliminationComponent.h"

#include "Engine/World.h"
#include "Player/ShooterPlayerState.h"
#include "GameFramework/Pawn.h"
#include "ShooterTypes/ShooterTypes.h"


UEliminationComponent::UEliminationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SequentialKillInterval = 2.f;
	LastKillTime = 0.f;
	SequentialKills = 0;
	Streak = 0;
	KillsNeededForStreak = 5;
}

void UEliminationComponent::OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal)
{
	AShooterPlayerState* AttackerPS = GetPlayerStateFromActor(Attacker);
	if (!IsValid(AttackerPS)) return;
	
	ProcessHitOrMiss(bHit, AttackerPS);
	
	if (!bHit) return;
	
	AShooterPlayerState* VictimPS = GetPlayerStateFromActor(Victim);
	if (!IsValid(VictimPS)) return;
	
	if (bLethal)
	{
		ProcessElimination(bHeadShot, AttackerPS, VictimPS);
	}
}

void UEliminationComponent::ProcessHitOrMiss(bool bHit, AShooterPlayerState* AttackerPS)
{
	if (bHit)
	{
		AttackerPS->AddHit();
	}
	else
	{
		AttackerPS->AddMiss();
	}
}

void UEliminationComponent::ProcessElimination(bool bHeadShot, AShooterPlayerState* AttackerPS,
	AShooterPlayerState* VictimPS)
{
	AttackerPS->AddScoredKills();
	VictimPS->AddDefeat();
	
	ESpecialElimType SpecialElimType{};
	
	ProcessHeadShot(bHeadShot, SpecialElimType, AttackerPS);
	ProcessSequentialElimination(SpecialElimType, AttackerPS);
	ProcessStreaks(SpecialElimType, AttackerPS, VictimPS);
}

void UEliminationComponent::ProcessHeadShot(bool bHeadShot, ESpecialElimType& OutElimType,
	AShooterPlayerState* AttackerPS)
{
	if (bHeadShot)
	{
		OutElimType |= ESpecialElimType::HeadShot;
		AttackerPS->AddHeadShotKill();
	}
}

void UEliminationComponent::ProcessSequentialElimination(ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	
	if (CurrentTime - LastKillTime <= SequentialKillInterval)
	{
		++SequentialKills;
	}
	else
	{
		SequentialKills = 1;
	}
	LastKillTime = CurrentTime;
	
	if (SequentialKills > 1)
	{
		OutElimType |= ESpecialElimType::Sequential;
		AttackerPS->AddSequentialKill(SequentialKills);
	}
}

void UEliminationComponent::ProcessStreaks(ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS,
	AShooterPlayerState* VictimPS)
{
	++Streak;
	
	if (Streak >= KillsNeededForStreak)
	{
		OutElimType |= ESpecialElimType::Streak;
		AttackerPS->SetOnStreak(true);
		AttackerPS->UpdateHighestStreak(Streak);
	}
	
	if (VictimPS->IsOnStreak())
	{
		OutElimType |= ESpecialElimType::Showstopper;
		AttackerPS->AddShowStopperKill();
		VictimPS->SetOnStreak(false);
	}
	
	if (AttackerPS->GetLastAttacker() == VictimPS)
	{
		OutElimType |= ESpecialElimType::Revenge;
		AttackerPS->AddRevengeKill();
		AttackerPS->SetLastAttacker(nullptr);
	}
	VictimPS->SetOnStreak(AttackerPS);
}

AShooterPlayerState* UEliminationComponent::GetPlayerStateFromActor(AActor* Actor)
{
	APawn* Pawn = Cast<APawn>(Actor);
	
	if (IsValid(Pawn))
	{
		return Pawn->GetPlayerState<AShooterPlayerState>();
	}
	return nullptr;
}

