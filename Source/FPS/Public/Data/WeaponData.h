// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class UAnimMontage;
class UBlendSpace;
class UAnimSequence;


USTRUCT(BlueprintType)
struct FRecoilStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float PitchMin = 1.f;

	UPROPERTY(EditAnywhere)
	float PitchMax = 1.f;

	UPROPERTY(EditAnywhere)
	float YawMin = 0.f;

	UPROPERTY(EditAnywhere)
	float YawMax = 0.f;
};

USTRUCT(BlueprintType)
struct FWeaponRecoil
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TArray<FRecoilStep> Pattern;
	
	UPROPERTY(EditAnywhere)
	float ResetDelay = 0.25f;
};

USTRUCT(BlueprintType)
struct FPlayerAnims
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> IdleAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> AimIdleAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> CrouchIdleAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> SprintAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> AimOffest_Hip;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> AimOffest_Aim;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> Strafe_Standing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> Strafe_Crouching;
};

USTRUCT(BlueprintType)
struct FMontageData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> EquipMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> ReloadMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FireMontage = nullptr;
};

UCLASS()
class FPS_API UWeaponData : public UDataAsset
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|WeaponData|Weapons")
	TMap<FGameplayTag, FName> GripPoints;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "FPS|WeaponData|Weapons|Weapons")
	TMap<FGameplayTag, FMontageData> WeaponMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "FPS|WeaponData|Weapons|FirstPerson")
	TMap<FGameplayTag, FPlayerAnims> FirstPersonAnims;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "FPS|WeaponData|Weapons|ThirdPerson")
	TMap<FGameplayTag, FPlayerAnims> ThirdPersonAnims;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS|WeaponData|Weapons|FOV")
	TMap<FGameplayTag, float> FOV;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "FPS|WeaponData|Weapons|FirstPerson")
	TMap<FGameplayTag, FMontageData> FirstPersonMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "FPS|WeaponData|Weapons|ThirdPerson")
	TMap<FGameplayTag, FMontageData> ThirdPersonMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|Weapons")
	TMap<FGameplayTag, FWeaponRecoil> WeaponRecoil;
};
