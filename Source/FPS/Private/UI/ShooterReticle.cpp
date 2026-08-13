// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterReticle.h"

#include "Character/ShooterCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Weapon/Weapon.h"

namespace Ammo
{
	const FName Rounds_Current = FName("Rounds_Current");
	const FName Rounds_Max = FName("Rounds_Max");
	const FName IsLowAmmo = FName("IsLowAmmo"); 
}

namespace  Reticle
{
	const FName RoundedCornerScale = FName("RoundedCornerScale");
	const FName ShapeCutThickness = FName("ShapeCutThickness");
	const FName Inner_RGBA = FName("Inner_RGBA");
}

void UShooterReticle::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Image_Reticle->SetRenderOpacity(0.f);
	Image_AmmoCounter->SetRenderOpacity(0.f);
	_BaseCornerScaleFactor_RoundFired = 0.f;
	_BaseShapeCutFactor_RoundFired = 0.f;
	_BaseCornerScaleFactor_Aiming = 0.f;
	_BaseShapeCutFactor_Aiming = 0.f;
	bAiming = false;
	bTargetingPlayer = false;
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(ShooterCharacter)) return;
	
	UCombatComponent* Combat = UCombatComponent::FindCombatComponent(ShooterCharacter);
	
	if (!IsValid(Combat)) return;
	
	OnPossessedPawnChanged(nullptr, ShooterCharacter);
	
	if (ShooterCharacter->HasWeaponFirstReplicated())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->ReticleParams, Combat->bHitPlayer);
			OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		}
	}
	else
	{
		ShooterCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
	}
	
	if (ShooterCharacter->HasAuthority())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->ReticleParams, Combat->bHitPlayer);
			OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		}
	}
}

void UShooterReticle::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	_BaseCornerScaleFactor_RoundFired = FMath::FInterpTo(_BaseCornerScaleFactor_RoundFired, 0.f, InDeltaTime, CurrentReticleParams.RoundFiredInterpSpeed);
	_BaseShapeCutFactor_RoundFired = FMath::FInterpTo(_BaseShapeCutFactor_RoundFired, 0.f, InDeltaTime, CurrentReticleParams.RoundFiredInterpSpeed);
	 
	_BaseCornerScaleFactor_Aiming = FMath::FInterpTo(_BaseCornerScaleFactor_Aiming, bAiming ? CurrentReticleParams.ScaleFactor_Aiming : CurrentReticleParams.ScaleFactor_NotAiming, InDeltaTime, CurrentReticleParams.AimingInterpSpeed);
	_BaseShapeCutFactor_Aiming = FMath::FInterpTo(_BaseShapeCutFactor_Aiming, bAiming ? CurrentReticleParams.ShapeCutFactor_Aiming : CurrentReticleParams.ShapeCutFactor_NotAiming, InDeltaTime, CurrentReticleParams.AimingInterpSpeed);
	
	_BaseCornerScaleFactor_TargetingPlayer = FMath::FInterpTo(_BaseCornerScaleFactor_TargetingPlayer, bTargetingPlayer ? CurrentReticleParams.ScaleFactor_Targeting : CurrentReticleParams.ScaleFactor_NotTargeting, InDeltaTime, CurrentReticleParams.TargetingPlayerInterpSpeed);
	
	BaseCornerScaleFactor = _BaseCornerScaleFactor_RoundFired + _BaseCornerScaleFactor_Aiming + _BaseCornerScaleFactor_TargetingPlayer;
	BaseShapeCutFactor = _BaseShapeCutFactor_RoundFired + _BaseShapeCutFactor_Aiming;
	
	if (CurrentReticle_DynMatInst.IsValid())
	{
		CurrentReticle_DynMatInst->SetScalarParameterValue(Reticle::RoundedCornerScale, BaseCornerScaleFactor);
		CurrentReticle_DynMatInst->SetScalarParameterValue(Reticle::ShapeCutThickness, BaseShapeCutFactor);
	}
}

void UShooterReticle::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* oldPawnCombat = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(oldPawnCombat))
	{
		oldPawnCombat->OnReticleChanged.RemoveDynamic(this, &ThisClass::OnReticleChanged);
		oldPawnCombat->OnAmmoCounterChanged.RemoveDynamic(this, &ThisClass::OnAmmoCounterChanged);
		oldPawnCombat->OnRoundFired.RemoveDynamic(this, &ThisClass::OnRoundFired);
		oldPawnCombat->OnAimingStatusChanged.RemoveDynamic(this, &ThisClass::OnAimingStatusChanged);
		oldPawnCombat->OnTargetingPlayerStatusChange.RemoveDynamic(this, &ThisClass::OnTargetingPlayerStatusChanged);
	}
	
	UCombatComponent* newPawnCombat = UCombatComponent::FindCombatComponent(NewPawn);
	if (IsValid(newPawnCombat))
	{
		Image_Reticle->SetRenderOpacity(1.f);
		Image_AmmoCounter->SetRenderOpacity(1.f);
		newPawnCombat->OnReticleChanged.AddDynamic(this, &ThisClass::OnReticleChanged);
		newPawnCombat->OnAmmoCounterChanged.AddDynamic(this, &ThisClass::OnAmmoCounterChanged);
		newPawnCombat->OnRoundFired.AddDynamic(this, &ThisClass::OnRoundFired);
		newPawnCombat->OnAimingStatusChanged.AddDynamic(this, &ThisClass::OnAimingStatusChanged);
		newPawnCombat->OnTargetingPlayerStatusChange.AddDynamic(this, &ThisClass::OnTargetingPlayerStatusChanged);
	}
}

void UShooterReticle::OnWeaponFirstReplicated(AWeapon* Weapon, bool bIsTargetingPlayer)
{
	OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->ReticleParams, bIsTargetingPlayer);
	OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
}

void UShooterReticle::OnReticleChanged(UMaterialInstanceDynamic* ReticleDynMatInst, const FReticleParams& ReticleParams, bool bCurrentlyTargetingPlayer)
{
	CurrentReticleParams = ReticleParams;
	CurrentReticle_DynMatInst = ReticleDynMatInst;
	
	FSlateBrush Brush;
	Brush.SetResourceObject(ReticleDynMatInst);
	if (IsValid(Image_Reticle))
	{
		Image_Reticle->SetBrush(Brush);
	}
	
	OnTargetingPlayerStatusChanged(bCurrentlyTargetingPlayer);
}

void UShooterReticle::OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynMatInst, int32 RoundsCurrent,
	int32 RoundMax)
{
	CurrentAmmo_DynMatInst = AmmoCounterDynMatInst;
	CurrentAmmo_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Current, RoundsCurrent);
	CurrentAmmo_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Max, RoundMax);
	
	FSlateBrush Brush;
	Brush.SetResourceObject(AmmoCounterDynMatInst);
	if (IsValid(Image_AmmoCounter))
	{
		Image_AmmoCounter->SetBrush(Brush);
	}
	bool bIsLowAmmo = RoundsCurrent <= 2;

	CurrentAmmo_DynMatInst->SetScalarParameterValue(
		Ammo::IsLowAmmo,
		bIsLowAmmo ? 1.0f : 0.0f
	);
}

void UShooterReticle::OnRoundFired(int32 RoundsCurrent, int32 RoundMax, int32 RoundsInReserve)
{
	_BaseCornerScaleFactor_RoundFired += CurrentReticleParams.ScaleFactor_RoundFired;
	_BaseShapeCutFactor_RoundFired += CurrentReticleParams.ShapeCutFactor_RoundFired;
	if (CurrentAmmo_DynMatInst.IsValid())
	{
		CurrentAmmo_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Current, RoundsCurrent);
		CurrentAmmo_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Max, RoundMax);
		
		bool bIsLowAmmo = RoundsCurrent <= 2;

		CurrentAmmo_DynMatInst->SetScalarParameterValue(
			Ammo::IsLowAmmo,
			bIsLowAmmo ? 1.0f : 0.0f
		);
	}
}

void UShooterReticle::OnAimingStatusChanged(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UShooterReticle::OnTargetingPlayerStatusChanged(bool bTargeting)
{
	bTargetingPlayer = bTargeting;
	if (CurrentReticle_DynMatInst.IsValid())
	{
		FLinearColor ReticleColor = bTargeting ? FLinearColor::Red : FLinearColor::White;
		CurrentReticle_DynMatInst->SetVectorParameterValue(Reticle::Inner_RGBA, ReticleColor);
	}
}
