#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchResultWidget.generated.h"

struct FScoreboardEntry;
class UTextBlock;
class UImage;
class UVerticalBox;
class UScoreboardRow;
class AShooterPlayerState;

UCLASS()
class FPS_API UMatchResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Result;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Result;

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerList;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|ScoreboardRow")
	TSubclassOf<UScoreboardRow> ScoreboardRowClass;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayMatchResultAnimation();
	
	
private:

	AShooterPlayerState* GetPlayerState() const;

	UFUNCTION()
	void OnMatchResultChanged(bool bWon, const TArray<FScoreboardEntry>& Entries);

	UFUNCTION()
	void OnPlayerStateReplicated();

	void RefreshResultScoreboard(const TArray<FScoreboardEntry>& Entries);
};