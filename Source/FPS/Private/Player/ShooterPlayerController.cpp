// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "SNodePanel.h"
#include "TimerManager.h"
#include "Game/ShooterGameStateBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/ShooterPlayerState.h"
#include "UI/GamePlay/Scoreboard.h"

AShooterPlayerController::AShooterPlayerController()
{
    bReplicates = true;
    bPawnAlive = true;
}

void AShooterPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    OnPlayerStateReplicated.Broadcast();
}

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if(IsValid(Subsystem))
    {
        Subsystem->AddMappingContext(ShooterIMC, 0);
    }
}

void AShooterPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* ShooterInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent); 
    ShooterInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
    ShooterInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
    ShooterInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Input_Jump);
    ShooterInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::Input_Crouch);
    ShooterInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Started, this, &ThisClass::Input_ScoreboardStarted);
    ShooterInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Completed, this, &ThisClass::Input_ScoreboardReleased);
}

void AShooterPlayerController::OnPossess(APawn* P)
{
    Super::OnPossess(P);
    bPawnAlive = true;
}

void AShooterPlayerController::Input_Crouch()
{
    if(!IsValid(GetCharacter()))
    {
        return;
    }
    if (!bPawnAlive) return;
    
    if(UCharacterMovementComponent* CMC = GetCharacter()->GetCharacterMovement(); IsValid(CMC))
    {
        CMC->bWantsToCrouch = !CMC->bWantsToCrouch;
    }
}

void AShooterPlayerController::Input_Jump()
{
    if(!IsValid(GetCharacter()))
    {
        return;
    }
    if (!bPawnAlive) return;
    
    UCharacterMovementComponent* CMC = GetCharacter()->GetCharacterMovement();
    if(!IsValid(CMC)) return;

    if(CMC->bWantsToCrouch)
    {
        CMC->bWantsToCrouch = false;
    }
    else
    {
        GetCharacter()->Jump();
    }
}

void AShooterPlayerController::Input_Move(const FInputActionValue& Value)
{
    if (!bPawnAlive) return;
    
    const FVector2D InputAxisVector = Value.Get<FVector2D>();
    const FRotator Rotation = GetControlRotation();
    const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    if(APawn* ControlledPawn = GetPawn())
    {
        ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
        ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
    }
}

void AShooterPlayerController::Input_Look(const FInputActionValue& Value)
{
    if (!bPawnAlive) return;
    
    const FVector2D InputAxisVector = Value.Get<FVector2D>();
    AddYawInput(InputAxisVector.X);
    AddPitchInput(InputAxisVector.Y);
}

void AShooterPlayerController::Input_ScoreboardStarted()
{
    if (!IsValid(ScoreboardWidgetClass))
    {
        return;
    }

    if (!IsValid(ScoreboardWidget))
    {
        ScoreboardWidget = CreateWidget<UScoreboard>(
            this,
            ScoreboardWidgetClass
        );

        if (IsValid(ScoreboardWidget))
        {
            ScoreboardWidget->AddToViewport(100);
            UScoreboard::CenterWidget(ScoreboardWidget);
        }
    }

    if (IsValid(ScoreboardWidget))
    {
        ScoreboardWidget->SetVisibility(ESlateVisibility::Visible);
    }
    
    UpdateScoreboard();

    GetWorldTimerManager().SetTimer(
        ScoreboardUpdateTimer,
        this,
        &ThisClass::UpdateScoreboard,
        0.2f,
        true
    );
}

void AShooterPlayerController::Input_ScoreboardReleased()
{
    GetWorldTimerManager().ClearTimer(ScoreboardUpdateTimer);

    if (IsValid(ScoreboardWidget))
    {
        ScoreboardWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void AShooterPlayerController::UpdateScoreboard()
{
    Server_RequestScoreboard();
}

void AShooterPlayerController::Server_RequestScoreboard_Implementation()
{
    AShooterGameStateBase* GameState = GetWorld()->GetGameState<AShooterGameStateBase>();

    if (!IsValid(GameState))
    {
        return;
    }

    TArray<FScoreboardEntry> Entries;

    for (APlayerState* PS : GameState->PlayerArray)
    {
        AShooterPlayerState* ShooterPS = Cast<AShooterPlayerState>(PS);

        if (!IsValid(ShooterPS))
        {
            continue;
        }

        FScoreboardEntry Entry;

        Entry.PlayerName = ShooterPS->GetPlayerName();
        Entry.Kills = ShooterPS->GetScoredKills();
        Entry.Deaths = ShooterPS->GetDefeats();
        Entry.Ping = ShooterPS->GetCompressedPing() * 4;

        Entries.Add(Entry);
    }
    
    if (IsLocalController())
    {
        if (IsValid(ScoreboardWidget))
        {
            ScoreboardWidget->RefreshScoreboard(Entries);
        }
        return;
    }
    
    Client_ReceiveScoreboard(Entries);
}

void AShooterPlayerController::Client_ReceiveScoreboard_Implementation(const TArray<FScoreboardEntry>& Entries)
{
    if (!IsValid(ScoreboardWidget))
    {
        return;
    }

    ScoreboardWidget->RefreshScoreboard(Entries);
}
