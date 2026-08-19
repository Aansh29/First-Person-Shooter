#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchResultWidget.generated.h"

class UTextBlock;
class UImage;
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

private:

	AShooterPlayerState* GetPlayerState() const;

	UFUNCTION()
	void OnMatchResultChanged(bool bWon);

	UFUNCTION()
	void OnPlayerStateReplicated();
};