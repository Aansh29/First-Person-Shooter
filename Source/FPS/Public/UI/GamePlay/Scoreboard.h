#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Scoreboard.generated.h"

struct FScoreboardEntry;
class UScoreboardRow;
class UVerticalBox;


UCLASS()
class FPS_API UScoreboard : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void RefreshScoreboard(const TArray<FScoreboardEntry>& Entries);

	UFUNCTION(BlueprintCallable)
	static void CenterWidget(UUserWidget* Widget);
	
protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerList;
	
		
	UPROPERTY(EditDefaultsOnly, Category = "FPS|ScoreboardRow")
	TSubclassOf<UScoreboardRow> ScoreboardRowClass;
};