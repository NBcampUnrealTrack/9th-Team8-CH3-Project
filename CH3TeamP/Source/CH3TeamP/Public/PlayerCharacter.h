#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputAction* AimAction;
	
	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float SprintSpeed = 900.f;
	
protected:
	virtual void BeginPlay() override;
	
	// 입력 함수
	void InputActionMove(const FInputActionValue& Value);
	void InputActionLook(const FInputActionValue& Value);
	void InputActionJump(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void Shoot();
	void StartAim();
	void StopAim();
	
};
	