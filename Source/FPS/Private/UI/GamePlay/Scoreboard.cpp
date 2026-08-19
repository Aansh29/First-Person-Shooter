#include "UI/GamePlay/Scoreboard.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "UI/GamePlay/ScoreboardRow.h"
#include "Components/VerticalBox.h"
#include "ShooterTypes/ShooterTypes.h"

void UScoreboard::NativeConstruct()
{
	Super::NativeConstruct();
}

void UScoreboard::RefreshScoreboard(const TArray<FScoreboardEntry>& Entries)
{
	if (!IsValid(PlayerList) || !IsValid(ScoreboardRowClass))
	{
		return;
	}

	PlayerList->ClearChildren();

	for (const FScoreboardEntry& Entry : Entries)
	{
		UScoreboardRow* Row = CreateWidget<UScoreboardRow>(
			GetWorld(),
			ScoreboardRowClass
		);

		if (!IsValid(Row))
		{
			continue;
		}

		Row->SetPlayerData(Entry);

		PlayerList->AddChild(Row);
	}
}

void UScoreboard::CenterWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return;
	}

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);

	const FVector2D CenterPosition(
		ViewportSize.X * 0.5f,
		ViewportSize.Y * 0.5f
	);

	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widget->SetPositionInViewport(CenterPosition, true);
}