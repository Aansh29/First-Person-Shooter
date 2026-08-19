// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameStateBase.generated.h"


class AShooterPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreInfoChanged, int32, CurrentMaxKill);

UCLASS()
class FPS_API AShooterGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AShooterGameStateBase();
	
	UPROPERTY(BlueprintAssignable)
	FScoreInfoChanged OnScoreInfoChanged;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastScoreInfo(int32 CurrentMaxKill);
	
	bool HasFirstBloodBeenHad() const;
	void UpdateLeader();
	AShooterPlayerState* GetSoleLeader() const;
	AShooterPlayerState* GetTopScorer() const;
	int32 GetTopScore() const;
	bool IsTiedForTheLead(AShooterPlayerState* PlayerState);
private:
	
	bool bHasFirstBloodBeenHad;
	
	UPROPERTY()
	TArray<TObjectPtr<AShooterPlayerState>> Leaders;
	
};
