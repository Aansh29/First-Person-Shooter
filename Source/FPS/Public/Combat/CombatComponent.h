// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/Actor.h"
#include "CombatComponent.generated.h"


class UMaterialInstanceDynamic;
struct FHitResult;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReticleChanged, UMaterialInstanceDynamic*, ReticleDynMatInst, const FReticleParams&, ReticleParams, bool, bCurrentlyTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAmmoCounterChanged, UMaterialInstanceDynamic*, AmmoCounterDynMatInst, int32, RoundsCurrent, int32, RoundsMax);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRoundFired, int32, RoundsCurrent, int32, RoundsMax, int32, RoundsInReserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingStatusChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingPlayerStatusChange, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCurrentReserveAmmoChanged, int32, RoundsInReserve, int32, RoundsInWeapon, UMaterialInterface*, WeaponIconMaterial);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FRoundReported, AActor*, Attacker, AActor*, Victim, bool, bHit, bool, bHeadShot, bool, bLethal);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category = "FPS|Combat")
	static UCombatComponent* FindCombatComponent(const AActor* Actor) { return ( IsValid(Actor) ? Actor->FindComponentByClass<UCombatComponent>() : nullptr ); }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS|Weapons")
	TObjectPtr<UWeaponData> WeaponData;
	
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();
	void InitalizeWeaponWidgets() const;
	void Notify_CycleWeapon();
	void Notify_ReloadWeapon();
	
	void AddAmmo(const FGameplayTag& WeaponType, int32 Amount);
	
	UPROPERTY(BlueprintAssignable)
	FReticleChanged OnReticleChanged;
	
	UPROPERTY(BlueprintAssignable)
	FAmmoCounterChanged OnAmmoCounterChanged;
	
	UPROPERTY(BlueprintAssignable)
	FRoundFired OnRoundFired;
	
	UPROPERTY(BlueprintAssignable)
	FAimingStatusChanged OnAimingStatusChanged;
	
	UPROPERTY(BlueprintAssignable)
	FTargetingPlayerStatusChange OnTargetingPlayerStatusChange;
	
	UPROPERTY(BlueprintAssignable)
	FCurrentReserveAmmoChanged OnCurrentReserveAmmoChanged;
	
	UPROPERTY(BlueprintAssignable)
	FRoundReported OnRoundReported;

	void Equip(AWeapon* Weapon);
	void EquipWeapon(AWeapon* Weapon);
	
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AWeapon* Weapon);
	
	void SpawnInventory();
	void DestroyInventory();
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bAiming;
	
	bool bHitPlayer;
	
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentReserveAmmo)
	int32 CurrentReserveAmmo;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapons")
	float TraceLength;
	
	UFUNCTION()
	void BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted);
private:
	
	TMap<FGameplayTag, int32> ReserveAmmo;
	
	bool bHitPlayerLastFrame;
	bool bTriggerPressed;
	FTimerHandle FireTimer;
	void FireTimerFinished();

	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);
	
	void SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon);
	
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& Hit);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(const FHitResult& Hit, int32 AuthAmmo);
	
	UFUNCTION()
	void OnRep_CurrentReserveAmmo();
	
	void Local_Aim(bool bPressed);
	
	void Local_FireWeapon();
	
	UPROPERTY(Transient, Replicated)
	TArray <AWeapon*> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClasses;
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
	int32 CurrentRecoilIndex = 0;

	FTimerHandle RecoilResetTimer;
	
	FVector2D CurrentRecoil;

	FVector2D TargetRecoil;
	
	void ApplyRecoil();

	void ResetRecoil();

	FRecoilStep GetCurrentRecoilStep() const;

	void AdvanceRecoil();
	
	int32 Local_WeaponIndex;
	int32 AdvanceWeaponIndex();
	
	void Local_CycleWeapon(int32 WeaponIndex);
	
	UFUNCTION(Server, Reliable)
	void Server_CycleWeapon(int32 WeaponIndex);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CycleWeapon(int32 WeaponIndex);
	
	void Local_ReloadWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_ReloadWeapon();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReloadWeapon();
	
	UFUNCTION(Client, Reliable)
	void Client_ReloadWeapon(int32 NewWeaponAmmo, int32 NewCarriedAmmo);
};
