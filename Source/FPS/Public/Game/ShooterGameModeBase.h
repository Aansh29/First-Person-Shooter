#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameModeBase.generated.h"

struct FScoreboardEntry;

UCLASS()
class FPS_API AShooterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AShooterGameModeBase();

	virtual void BeginPlay() override;
	void FinishMatch();
	void RequestRespawn(ACharacter* Character, AController* Controller);
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Match")
	int32 TargetKills;
	
protected:

	UPROPERTY(EditDefaultsOnly, Category="FPS|Match")
	float MatchDuration = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="FPS|Match")
	float ResultsDisplayTime = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Match")
	FName MatchMap;

	FTimerHandle MatchTimerHandle;
	FTimerHandle MatchTimeBroadcastTimerHandle;
	FTimerHandle ResultsTimerHandle;

	void StartMatchTimer();
	void BroadcastMatchTime();
	void RestartMatch();

	float GetRemainingMatchTime() const;
	
private:
	bool bMatchFinished;
};