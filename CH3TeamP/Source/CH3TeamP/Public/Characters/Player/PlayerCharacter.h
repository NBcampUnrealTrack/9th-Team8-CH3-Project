#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Weapons/WeaponDataAsset.h"
#include "Camera/CameraComponent.h"
#include "Types/CombatTypes.h"
#include "PlayerCharacter.generated.h"

class UCH3StaminaComponent;

UCLASS()
class CH3TEAMP_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	class UCameraComponent* FirstPersonCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	USkeletalMeshComponent* ThirdPersonMesh;
	
		
	// 입력 에셋
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UPlayerInputConfigDataAsset* PlayerInputConfig;

	void UpdateMoveSpeed();
	
	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float SprintSpeed = 900.f;
	
	UPROPERTY(EditAnywhere, Category="Movement")
	float HitSpeed = 300.f;
	
	bool bIsSprinting;
	bool bIsHit;
	bool bIsAiming = false;
	
	// 손에 든 무기 메시 (단일 슬롯, 무기 전환 시 SkeletalMesh 교체)
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	USkeletalMeshComponent* EquippedWeaponMesh;
	
	// 무기 DataAsset 3종 — BP에서 할당
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	class UWeaponDataAsset* RipleWeaponData;
	
	// 현재 장착 중인 무기 종류
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	EWeaponType CurrentWeapon = EWeaponType::RipleGun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	class UHealthComponent* HealthComp;
	
	// 총구 머즐 플래시 (무기 메시 "Muzzle" 소켓에 부착)
	UPROPERTY(VisibleAnywhere, Category="Effect")
	class UNiagaraComponent* MuzzleComp;

	// 임팩트 이펙트 — Cascade 파티클 (BP에서 할당)
	UPROPERTY(EditAnywhere, Category="Effect")
	class UParticleSystem* ImpactEffect;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stamina")
	UCH3StaminaComponent* StaminaComp;
	
	UPROPERTY(EditAnywhere, Category="Weapon")
	int32 MaxAmmo = 30;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	int32 CurrentAmmoCount  = 30;

	UPROPERTY(EditAnywhere, Category="Weapon")
	float ReloadTime = 2.f;
	
	UFUNCTION(BlueprintCallable, Category="Damage")
	void OnDamage(int32 Amount);

	bool bIsReloading = false;
	
	FTimerHandle ReloadTimerHandle;
	
	float GetCurrentDamage() const;

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	EAmmoType CurrentAmmoType = EAmmoType::Normal;	

	// 입력 함수
	void InputActionMove(const FInputActionValue& Value);
	void InputActionLook(const FInputActionValue& Value);
	void InputActionJump(const FInputActionValue& Value);
	void InputActionCrouch(const FInputActionValue& Value);
	void InputActionUnCrouch(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void FireGun();
	void StartFire();
	void StopFire();
	void Reload(const FInputActionValue& Value);
	void FinishReload();
	void ToggleAim();
	void OnHit();
	void EndHit();
	
	// 무기 장착 처리
	void TakeRipleGun();
    
	void EquipWeapon(class UWeaponDataAsset* WeaponData);

	// 입력 핸들러
	void InputActionRiple(const struct FInputActionValue& Value);
	
	void InputActionFire(const struct FInputActionValue& Value);
	
	// 착지 시각 기록 → 착지 직후 0.2초 점프 잠금
	virtual void Landed(const FHitResult& Hit) override;

	float LastLandedTime = -10.f;
	
	UPROPERTY(EditAnywhere, Category="Weapon|Damage")
	float NormalDamage = 30.f;

	UPROPERTY(EditAnywhere, Category="Weapon|Damage")
	float PiercingDamage = 25.f;

	UPROPERTY(EditAnywhere, Category="Weapon|Damage")
	float ExplosiveDamage = 20.f;
	

	
private:

	float StandingCameraHeight;
	float CrouchCameraHeight = 32.f;

	float TargetCameraHeight = 64.f;
	float CameraInterpSpeed = 12.f;

	// 기본 시야각
	UPROPERTY(EditAnywhere, Category="Camera")
	float DefaultFOV = 90.f;

	// 조준 시 시야각
	UPROPERTY(EditAnywhere, Category="Camera")
	float AimFOV = 65.f;

	// 현재 FOV
	float CurrentFOV;

	// 목표 FOV
	float TargetFOV;

	// FOV 변경 속도
	UPROPERTY(EditAnywhere, Category="Camera")
	float AimInterpSpeed = 15.f;
	
	void OnFireAnimation();
	void OnHitAnimation();
	void OnDeathAnimation();

	// 테스트용
	void InputActionTestDamage(const struct FInputActionValue& Value);
	void InputActionTestDeath(const struct FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category="Weapon")
	float FireRate = 8.f;

	FTimerHandle FireTimerHandle;
	
	void FireNormal();
	void FirePiercing();
	void FireExplosive();

	
	
};
	