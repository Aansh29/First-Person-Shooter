#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoundWidget.generated.h"

class UTextBlock;
class AShooterPlayerState;

UCLASS()
class FPS_API URoundWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Round;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CurrentMaxKill;

private:

	UFUNCTION()
	void OnPlayerStateReplicated();

	UFUNCTION()
	void SetMatchTime(int32 RemainingTime);
	
	UFUNCTION()
	void SetScoreInfo(int32 CurrentMaxKill);

	void UpdateLocalTimer();

	AShooterPlayerState* GetPlayerState() const;

	FTimerHandle LocalTimerHandle;

	int32 CurrentMatchTime = 0;
};