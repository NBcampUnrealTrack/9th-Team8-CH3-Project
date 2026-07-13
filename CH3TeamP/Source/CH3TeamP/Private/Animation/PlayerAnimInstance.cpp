#include "Animation/PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Owner = TryGetPawnOwner();
	if (!Owner) return;
	
	// 1. 캐릭터의 3차원 속도 가져오기
    const FVector Velocity = Owner->GetVelocity();
	HDirectionSpeed = FVector::DotProduct(Velocity, Owner->GetActorRightVector());   // 좌우
	VDirectionSpeed = FVector::DotProduct(Velocity, Owner->GetActorForwardVector()); // 앞뒤
	
	VerticalSpeed = Velocity.Z; // 위(+)/아래(-)
	
	if (ACharacter* Ch = Cast<ACharacter>(Owner))
	{
		bIsFalling = Ch->GetCharacterMovement()->IsFalling();
		bIsCrouching = Ch->GetCharacterMovement()->IsCrouching();
	}
	
	
	
}
