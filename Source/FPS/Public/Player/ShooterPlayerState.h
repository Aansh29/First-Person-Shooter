// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/SpecialElimData.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"


class USpecialElim;
class USpecialElimData;
enum class ESpecialElimType : uint16;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchTimeChanged, int32, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchResultChanged, bool, bWon);

UCLASS()
class FPS_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AShooterPlayerState();
	
	UPROPERTY(BlueprintAssignable)
	FScoreChanged OnScoreChanged;
	
	UPROPERTY(BlueprintAssignable)
	FMatchTimeChanged OnMatchTimeChanged;
	
	void AddScoredKills();
	void AddDefeat();
	void AddHit();
	void AddMiss();
	void AddHeadShotKill();
	void AddSequentialKill(int32 SequenceCount);
	void UpdateHighestStreak(int32 StreakCount);
	void AddRevengeKill();
	void AddDethroneKill();
	void AddShowStopperKill();
	void GotFirstBlood();
	void IsNowWinner();
	void SetOnStreak(bool bIsOnStreak);
	void SetLastAttacker(APlayerState* Attacker);
	
	APlayerState* GetLastAttacker() const;
	bool IsOnStreak() const;
	int32 GetScoredKills() const;
	int32 GetDefeats() const;
	
	UFUNCTION(Client, Reliable)
	void Client_LostTheLead();
	
	UFUNCTION(Client, Reliable)
	void Client_ScoredKill(int32 KillScore);
	
	UFUNCTION(Client, Reliable)
	void Client_SpecialKill(const ESpecialElimType& SpecialElim, int32 SequentialKillCount, int32 StreakCount, int32 KillScore);
	
	UFUNCTION(Client, Reliable)
	void Client_MatchTimeChanged(int32 RemainingTime);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS|SpecialElims")
	TObjectPtr<USpecialElimData> SpecialElimData;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|SpecialElims")
	TSubclassOf<USpecialElim> SpecialElimWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|SpecialElims")
	float ElimDisplayTime;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|MatchTIme")
	int32 CurrentMatchTime = 120;
	int32 GetCurrentMatchTime() const { return CurrentMatchTime; }
	
	UPROPERTY(BlueprintAssignable)
	FMatchResultChanged OnMatchResultChanged;

	UFUNCTION(Client, Reliable)
	void Client_MatchResult(bool bWon);
	
private:
	int32 ScoredKills;
	int32 Defeats;
	int32 Hits;
	int32 Misses;
	
	bool bOnStreak;
	
	int32 HeadShotKills;
	
	TMap<int32, int32> SequentialKills;
	int32 HighestStreak;
	int32 RevengeKills;
	int32 DethroneKills;
	int32 ShowStopperKills;
	bool bFirstBlood;
	bool bWinner;
	
	TWeakObjectPtr<APlayerState> LastAttacker;
	
	TArray<ESpecialElimType> DecodeElimBitMask(ESpecialElimType ElimTypeBitmask);
	void ProcessNextSpecialElim();
	void ShowSpecialElim(const FSpecialElimInfo& ElimInfo);
	TQueue<FSpecialElimInfo> SpecialElimQueue;
	bool bIsProcessingQueue;
};
