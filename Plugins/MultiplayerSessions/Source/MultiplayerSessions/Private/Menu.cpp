// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
    PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);

    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;

    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }
    
    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
    }
}


bool UMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
    }

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
    }

    return true;
}

void UMenu::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);
    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);

    }
}

void UMenu::JoinButtonClicked()
{
    JoinButton->SetIsEnabled(false);
    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10000);
    }
}

void UMenu::MenuTearDown()
{
    RemoveFromParent();

    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeGameOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}
void UMenu::NativeDestruct()
{
    MenuTearDown();
    Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        // if (GEngine)
        // {
        //     GEngine->AddOnScreenDebugMessage(
        //         -1,
        //         15.f,
        //         FColor::Green,
        //         TEXT("Session created successfully!")
        //     );
        // }
        // UE_LOG(LogTemp, Warning, TEXT("Travelling to: %s"), *PathToLobby);
        //
        // GEngine->AddOnScreenDebugMessage(
        //     -1,
        //     10.f,
        //     FColor::Green,
        //     PathToLobby
        // );
        
        UWorld* World = GetWorld();
        if(World)
        {
            World->ServerTravel(PathToLobby);
        }
    }
    else
    {
        // if (GEngine)
        // {
        //     GEngine->AddOnScreenDebugMessage(
        //         -1,
        //         15.f,
        //         FColor::Red,
        //         TEXT("Failed to create session.")
        //     );
        // }
        HostButton->SetIsEnabled(true);
        JoinButton->SetIsEnabled(true);
    }
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
  if (MultiplayerSessionsSubsystem == nullptr || !bWasSuccessful || SessionResults.Num() == 0)
    {
        JoinButton->SetIsEnabled(true);
        return;
    }

    for (FOnlineSessionSearchResult Result : SessionResults)
    {
        FString SettingsValue;
        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        Result.Session.SessionSettings.NumPublicConnections = NumPublicConnections;
        Result.Session.SessionSettings.bAllowJoinInProgress = true;
        Result.Session.SessionSettings.bAllowJoinViaPresence = true;
        Result.Session.SessionSettings.bShouldAdvertise = true;
        Result.Session.SessionSettings.bUsesPresence = true;
        Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;

        if (SettingsValue == MatchType)
        {
            // if (GEngine)
            // {
            //     GEngine->AddOnScreenDebugMessage(
            //         -1,
            //         15.f,
            //         FColor::Green,
            //         FString(TEXT("Session Found!"))
            //     );
            // }

            MultiplayerSessionsSubsystem->JoinSession(Result);
            return;
        }
    }

    if(!bWasSuccessful || SessionResults.Num() == 0)
    {
        JoinButton->SetIsEnabled(true);
    }
}


void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (GEngine)
    {
        FString ResultString;
        switch (Result)
        {
            case EOnJoinSessionCompleteResult::Success: ResultString = "Success"; break;
            case EOnJoinSessionCompleteResult::AlreadyInSession: ResultString = "AlreadyInSession"; break;
            case EOnJoinSessionCompleteResult::SessionIsFull: ResultString = "SessionIsFull"; break;
            case EOnJoinSessionCompleteResult::SessionDoesNotExist: ResultString = "DoesNotExist"; break;
            default: ResultString = "UnknownError"; break;
        }
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("JoinSession result: %s"), *ResultString));
    }

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FString Address;
            if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
            {
                //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Resolved connect string: %s"), *Address));

                APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
                if (PlayerController)
                {
                    PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
                }
            }
            // else
            // {
            //     GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Failed to resolve connect string."));
            // }
        }
    }

    if(Result != EOnJoinSessionCompleteResult::Success)
    {
        JoinButton->SetIsEnabled(true);
    }
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    // Implementation for when a session is destroyed
}
void UMenu::OnStartSession(bool bWasSuccessful)
{
    // Implementation for when a session is started
}
