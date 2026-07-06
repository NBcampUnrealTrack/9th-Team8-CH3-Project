#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "WeaponDataAsset.h"
#include "PlayerCharacter.generated.h"

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
		
	// 입력 에셋
public:
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UPlayerInputConfigDataAsset* PlayerInputConfig;
	
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, Category="Input")
	class UInputAction* AimAction;
	
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
	
	// 손에 든 무기 메시 (단일 슬롯, 무기 전환 시 SkeletalMesh 교체)
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	USkeletalMeshComponent* EquippedWeaponMesh;
	
	// 무기 DataAsset 3종 — BP에서 할당
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	class UWeaponDataAsset* RipleWeaponData;
	
	// 현재 장착 중인 무기 종류
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	EWeaponType CurrentWeapon = EWeaponType::RipleGun;

protected:
	virtual void BeginPlay() override;
	
	// 입력 함수
	void InputActionMove(const FInputActionValue& Value);
	void InputActionLook(const FInputActionValue& Value);
	void InputActionJump(const FInputActionValue& Value);
	void InputActionCrouch(const FInputActionValue& Value);
	void InputActionUnCrouch(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void Shoot();
	void StartAim();
	void StopAim();

	void OnHit();
	void EndHit();
	
	// 무기 장착 처리
	void TakeRipleGun();
    
	void EquipWeapon(class UWeaponDataAsset* WeaponData);

	// 입력 핸들러
	void InputActionRiple(const struct FInputActionValue& Value);
	
	void FireGun();
	void InputActionFire(const struct FInputActionValue& Value);
	
};
	