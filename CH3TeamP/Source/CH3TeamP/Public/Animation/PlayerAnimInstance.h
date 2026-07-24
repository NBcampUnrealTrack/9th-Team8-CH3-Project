#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

UCLASS()
class CH3TEAMP_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 이동 값 — BlendSpace에서 사용
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float HDirectionSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float VDirectionSpeed = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsFalling = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsCrouching = false;
	
	// 점프 상승(+)/하강(-) 구분용 
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float VerticalSpeed = 0.0f;

	// 몽타주 브리지 — C++ 호출, AnimBP 구현
	UFUNCTION(BlueprintImplementableEvent, Category="Montage")
	void PlayFireMontage();

	UFUNCTION(BlueprintImplementableEvent, Category="Montage")
	void PlayDamageMontage(FName SectionName);

	UFUNCTION(BlueprintImplementableEvent, Category="Montage")
	void PlayDeathMontage(FName SectionName);
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	
};
