#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CH3GameplayTypes.h" 
#include "InputActionValue.h"
#include "Weapons/WeaponDataAsset.h"
#include "Camera/CameraComponent.h"
#include "Types/CombatTypes.h"
#include "Sound/SoundBase.h"
#include "PlayerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEXPChangedSignature, float, NewCurrentEXP, float, NewMaxEXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpSignature, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealtSignature,
float, DamageAmount, FVector, HitLocation, bool, bIsCritical);

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
	
	UPROPERTY(EditAnywhere, Category="Weapon")
	int32 MaxAmmo = 30;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	int32 CurrentAmmoCount  = 30;
	
	// 총구 머즐 플래시 (무기 메시 "Muzzle" 소켓에 부착)
	UPROPERTY(VisibleAnywhere, Category="Effect")
	class UNiagaraComponent* MuzzleComp;

	// 임팩트 이펙트 — Cascade 파티클 (BP에서 할당)
	UPROPERTY(EditAnywhere, Category="Effect")
	class UParticleSystem* ImpactEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	UParticleSystem* ExplosionImpactEffect;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stamina")
	UCH3StaminaComponent* StaminaComp;
	
	UFUNCTION(BlueprintCallable, Category="Damage")
	void OnDamage(int32 Amount);
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* ExplosionSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* ReloadSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ReloadAnimation")
	UAnimMontage* ReloadMontage;
	
	void PlayReloadMontage();
	
	bool bIsReloading = false;
	
	FTimerHandle ReloadTimerHandle;
	
	float GetCurrentDamage() const;
	
	
	// UI 담당자가 EXP 바/레벨 표시 갱신에 바인딩할 델리게이트
	
	/** 경험치 변경 시 방송 (NewCurrentEXP, NewMaxEXP) -> UI 담당: EXP 바 갱신용 */
	UPROPERTY(BlueprintAssignable, Category = "Level System|Events")
	FOnEXPChangedSignature OnEXPChanged;

	/** 레벨업 시 방송 (NewLevel) -> UI 담당: 화면 레벨 텍스트 갱신용 */
	UPROPERTY(BlueprintAssignable, Category = "Level System|Events")
	FOnLevelUpSignature OnLevelUp;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDamageDealtSignature OnDamageDealt; 
	
	// 경험치(레벨업)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level System")
	int32 CurrentLevel = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level System")
	float CurrentEXP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
	float MaxEXP = 100.f; // 다음 레벨까지 필요한 EXP

	// 경험치 획득 함수
	UFUNCTION(BlueprintCallable, Category = "Level System")
	void AddEXP(float EXPValue);

	// 레벨업 시 재생할 사운드 & 파티클 (에디터 지정)
	UPROPERTY(EditAnywhere, Category = "Level System|Effects")
	USoundBase* LevelUpSound;

	// GameMode의 OnUpgradeConfirmed 델리게이트에 바인딩할 함수
	UFUNCTION()
	void ApplyUpgrade(EUpgradeType ChosenUpgrade, AController* ForPlayer);
	

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
	void InputActionFire(const FInputActionValue& Value);

	// 착지 시각 기록 → 착지 직후 0.2초 점프 잠금
	virtual void Landed(const FHitResult& Hit) override;

	float LastLandedTime = -10.f;
	
	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction*  TestDamageAction;

	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction*  TestDeathAction;
	
	void OnFireAnimation();
	void OnHitAnimation();
	void OnDeathAnimation();

	void InputActionTestDamage(const struct FInputActionValue& Value);
	void InputActionTestDeath(const struct FInputActionValue& Value);
	
	// --- 캐릭터 전투 및 스탯 변수 ---
	UPROPERTY(EditAnywhere, Category="Weapon")
	float FireRate = 8.f;
	
	UPROPERTY(EditAnywhere, Category="Weapon|Damage")
	float NormalDamage = 30.f;

	UPROPERTY(EditAnywhere, Category="Weapon|Damage")
	float PiercingDamage = 25.f;

	UPROPERTY(EditAnywhere, Category="Weapon|Damage")
	float ExplosiveDamage = 20.f;


	UPROPERTY(EditAnywhere, Category="Weapon")
	float ReloadTime = 2.f;
	
	// 장전 속도 강화를 위한 몽타주 PlayRate 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadAnimation")
	float ReloadAnimSpeed = 1.0f;
	
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
	float AimFOV = 45.f;

	// 현재 FOV
	float CurrentFOV;

	// 목표 FOV
	float TargetFOV;

	// FOV 변경 속도
	UPROPERTY(EditAnywhere, Category="Camera")
	float AimInterpSpeed = 15.f;


	FTimerHandle FireTimerHandle;
	
	void FireNormal();
	void FirePiercing();
	void FireExplosive();

	
	
};
	