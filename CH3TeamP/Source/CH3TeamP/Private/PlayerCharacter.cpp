#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head")); // 머리 소켓 이름
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	FirstPersonCamera->SetRelativeLocation(FVector(10.f, 0.f, 0.f));
	
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	
}
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetMesh()->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
	
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	
}

void APlayerCharacter::Jump(const FInputActionValue& Value)
{
	
}

void APlayerCharacter::StartSprint()
{
	
}

void APlayerCharacter::StopSprint()
{
	
}

void APlayerCharacter::Shoot()
{
	
}

void APlayerCharacter::StartAim()
{
	
}

void APlayerCharacter::StopAim()
{
	
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

