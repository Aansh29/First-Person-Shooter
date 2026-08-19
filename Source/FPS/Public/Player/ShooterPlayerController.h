// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UScoreboard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerStateReplicated);

UCLASS()
class FPS_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();
	
	UPROPERTY(BlueprintAssignable)
	FPlayerStateReplicated OnPlayerStateReplicated;
	
	virtual void OnRep_PlayerState() override;
	
	bool bPawnAlive;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	virtual void OnPossess(APawn* P) override;
private:

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputMappingContext> ShooterIMC;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> ScoreboardAction;

	UPROPERTY(EditAnywhere, Category = "FPS|UI")
	TSubclassOf<UScoreboard> ScoreboardWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UScoreboard> ScoreboardWidget;

	void Input_Crouch();
	void Input_Jump();
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_ScoreboardStarted();
	void Input_ScoreboardReleased();
	
	FTimerHandle ScoreboardUpdateTimer;

	void UpdateScoreboard();
	
	UFUNCTION(Server, Reliable)
	void Server_RequestScoreboard();
	
	UFUNCTION(Client, Reliable)
	void Client_ReceiveScoreboard(const TArray<FScoreboardEntry>& Entries);
};
