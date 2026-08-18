// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerState.h"

#include "TimerManager.h"
#include "Data/SpecialElimData.h"
#include "UI/Elims/SpecialElim.h"

AShooterPlayerState::AShooterPlayerState()
{
	SetNetUpdateFrequency(100.f);
	
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
	bIsProcessingQueue = false;
	ElimDisplayTime = 0.5f;
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

int32 AShooterPlayerState::GetScoredKills() const
{
	return ScoredKills;
}

TArray<ESpecialElimType> AShooterPlayerState::DecodeElimBitMask(ESpecialElimType ElimTypeBitmask)
{
	TArray<ESpecialElimType> ValidElims;
	
	uint16 BitMaskValue = static_cast<uint16>(ElimTypeBitmask);
	
	for (uint16 i = 0; i < 16; i++)
	{
		if (BitMaskValue & (1 << i))
		{
			ESpecialElimType EnumValue = static_cast<ESpecialElimType>(1 << i);
			ValidElims.Add(EnumValue);
		}
	}
	return ValidElims;
}

void AShooterPlayerState::Client_ScoredKill_Implementation(int32 KillScore)
{
	OnScoreChanged.Broadcast(KillScore);
}

void AShooterPlayerState::Client_SpecialKill_Implementation(const ESpecialElimType& SpecialElim,
	int32 SequentialKillCount, int32 StreakCount, int32 KillScore)
{
	ensure(IsValid(SpecialElimData));
	
	OnScoreChanged.Broadcast(KillScore);
	
	TArray<ESpecialElimType> ElimTypes = DecodeElimBitMask(SpecialElim);
	for (ESpecialElimType ElimType : ElimTypes)
	{
		FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialElimInfo.FindChecked(ElimType);
		if (ElimType == ESpecialElimType::Sequential)
		{
			ElimMessageInfo.SequentialElimCount = SequentialKillCount;
		}
		
		if (ElimType == ESpecialElimType::Streak)
		{
			ElimMessageInfo.StreakCount = StreakCount;
		}
		ElimMessageInfo.ElimType = ElimType;
		SpecialElimQueue.Enqueue(ElimMessageInfo);
	}
	
	if (!bIsProcessingQueue)
	{
		ProcessNextSpecialElim();
	}
}

void AShooterPlayerState::ProcessNextSpecialElim()
{
	FSpecialElimInfo ElimInfo;
	
	if (SpecialElimQueue.Dequeue(ElimInfo))
	{
		bIsProcessingQueue = true;
		ShowSpecialElim(ElimInfo);
		
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AShooterPlayerState::ProcessNextSpecialElim, ElimDisplayTime, false);
		});
	}
	else
	{
		bIsProcessingQueue = false;
	}
}

void AShooterPlayerState::ShowSpecialElim(const FSpecialElimInfo& ElimInfo)
{
	FString ElimMessageString = ElimInfo.ElimMessage;
	if (ElimInfo.ElimType == ESpecialElimType::Sequential)
	{
		if (ElimInfo.SequentialElimCount == 2) ElimMessageString = FString("Double Kill!");
		else if (ElimInfo.SequentialElimCount == 3) ElimMessageString = FString("Triple Kill!");
		else if (ElimInfo.SequentialElimCount == 4) ElimMessageString = FString("Quad Kill!");
		else if (ElimInfo.SequentialElimCount > 4) ElimMessageString = FString::Printf(TEXT("Rampage x%d!"), ElimInfo.SequentialElimCount);
	}
	if (ElimInfo.ElimType == ESpecialElimType::Streak) ElimMessageString = FString::Printf(TEXT("Streak x%d!"), ElimInfo.StreakCount);
	
	if (IsValid(SpecialElimWidgetClass))
	{
		USpecialElim* ElimWidget = CreateWidget<USpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(ElimWidget))
		{
			ElimWidget->InitializeWidget(ElimMessageString, ElimInfo.ElimIcon);
			ElimWidget->AddToViewport();
		}
	}
}

void AShooterPlayerState::Client_LostTheLead_Implementation()
{
	ensure(IsValid(SpecialElimData));
	FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialElimInfo.FindChecked(ESpecialElimType::LostTheLead);
	
	if (IsValid(SpecialElimWidgetClass))
	{
		USpecialElim* ElimWidget = CreateWidget<USpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(ElimWidget))
		{
			ElimWidget->InitializeWidget(ElimMessageInfo.ElimMessage, ElimMessageInfo.ElimIcon);
			ElimWidget->AddToViewport();
		}
	}
}
