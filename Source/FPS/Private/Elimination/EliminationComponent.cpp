// Fill out your copyright notice in the Description page of Project Settings.


#include "Elimination/EliminationComponent.h"

#include "Engine/World.h"
#include "Game/ShooterGameModeBase.h"
#include "Game/ShooterGameStateBase.h"
#include "Player/ShooterPlayerState.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
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
	
	AShooterGameModeBase* GameMode = Cast<AShooterGameModeBase>(UGameplayStatics::GetGameMode(AttackerPS));
	
	if (AttackerPS->GetScoredKills() >= GameMode->TargetKills && IsValid(GameMode))
	{
		GameMode->FinishMatch();
	}
	
	ESpecialElimType SpecialElimType{};
	
	ProcessHeadShot(bHeadShot, SpecialElimType, AttackerPS);
	ProcessSequentialElimination(SpecialElimType, AttackerPS);
	ProcessStreaks(SpecialElimType, AttackerPS, VictimPS);
	
	AShooterGameStateBase* GameState = Cast<AShooterGameStateBase>(UGameplayStatics::GetGameState(AttackerPS));
	if (IsValid(GameState))
	{
		HandleFirstBlood(GameState, SpecialElimType, AttackerPS);
		UpdateLeaderStatus(GameState, SpecialElimType, AttackerPS, VictimPS);
		int32 MaxScore = FMath::Clamp(GameState->GetTopScore(), 0, GameMode->TargetKills);
		GameState->MulticastScoreInfo(MaxScore);
	}
	
	if (HasSpecialElimTypes(SpecialElimType))
	{
		AttackerPS->Client_SpecialKill(SpecialElimType, SequentialKills, Streak, AttackerPS->GetScoredKills());
	}
	else
	{
		AttackerPS->Client_ScoredKill(AttackerPS->GetScoredKills());
	}
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
	VictimPS->SetLastAttacker(AttackerPS);
}

void UEliminationComponent::HandleFirstBlood(AShooterGameStateBase* GameState, ESpecialElimType& OutElimType,
	AShooterPlayerState* AttackerPS)
{
	if (!GameState->HasFirstBloodBeenHad())
	{
		OutElimType |= ESpecialElimType::FirstBlood;
		AttackerPS->GotFirstBlood();
	}
}

void UEliminationComponent::UpdateLeaderStatus(AShooterGameStateBase* GameState, ESpecialElimType& OutElimType,
	AShooterPlayerState* AttackerPS, AShooterPlayerState* VictimPS)
{
	AShooterPlayerState* LastLeader = GameState->GetSoleLeader();
	const bool bAttackerWasTiedForTheLead = GameState->IsTiedForTheLead(AttackerPS);
	GameState->UpdateLeader();
	if (!bAttackerWasTiedForTheLead && GameState->IsTiedForTheLead(AttackerPS))
	{
		// Attacker was not tied for lead earlier and is now tied for the lead
		OutElimType |= ESpecialElimType::TiedTheLeader;
	}
	if (IsValid(LastLeader) && LastLeader != GameState->GetSoleLeader())
	{
		// Last leader has lost the lead
		LastLeader->Client_LostTheLead();
		
		if (VictimPS == LastLeader)
		{
			OutElimType |= ESpecialElimType::Dethrone;
			AttackerPS->AddDethroneKill();
		}
	}
	
	if (AttackerPS != LastLeader && AttackerPS == GameState->GetSoleLeader())
	{
		OutElimType |= ESpecialElimType::GainedTheLead;
	}
}

bool UEliminationComponent::HasSpecialElimTypes(const ESpecialElimType& SpecialElimType) const
{
	return static_cast<uint16>(SpecialElimType) != 0;
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

