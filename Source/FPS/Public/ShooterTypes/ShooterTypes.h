#pragma once

#include "ShooterTypes.generated.h"

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	Left UMETA(DisplayName = "TurningLeft"), 
	Right UMETA(DisplayName = "TurningRight"), 
	NotTurning UMETA(DisplayName = "NotTurning"),
};

USTRUCT(BlueprintType)
struct FReticleParams
{
	GENERATED_BODY()
	
	// Shape Cut Factor
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_RoundFired = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_NotAiming = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_Aiming = 0.f;
	
	//Scale Factor
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_RoundFired = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Aiming = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_NotAiming = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Targeting = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_NotTargeting = 0.f;
	
	// Interp Speeds
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RoundFiredInterpSpeed = 20.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AimingInterpSpeed = 15.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TargetingPlayerInterpSpeed = 10.f;
};

UENUM(meta = (BitFlags))
enum class ESpecialElimType : uint16
{
	None = 0,
	HeadShot = 1 << 0,
	Sequential = 1 << 1,
	Streak = 1 << 2,
	Revenge = 1 << 3,
	Dethrone = 1 << 4,
	Showstopper = 1 << 5,
	FirstBlood = 1 << 6,
	GainedTheLead = 1 << 7,
	TiedTheLeader = 1 << 8,
	LostTheLead = 1 << 9
};

ENUM_CLASS_FLAGS(ESpecialElimType)

USTRUCT(BlueprintType)
struct FScoreboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RevengeKills = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;
};