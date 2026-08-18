// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

class UTextBlock;
class AShooterPlayerState;

UCLASS()
class FPS_API UScoreWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Score;
	
private:
	
	AShooterPlayerState* GetPlayerState() const;
	
	UFUNCTION()
	void OnScoreChanged(int32 Score);
	
	UFUNCTION()
	void OnPlayerStateReplicated();
};
