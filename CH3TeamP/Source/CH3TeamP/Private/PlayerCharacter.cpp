#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h" // UEnhancedInputComponent, BindAction()
#include "EnhancedInputSubsystems.h" // UEnhancedInputLocalPlayerSubsystem, AddMappingContext
#include "InputActionValue.h" // FInputActionValue
#include "PlayerInputConfigDataAsset.h" 
#include "WeaponDataAsset.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h" // APlayerController
#include "Engine/LocalPlayer.h" // ULocalPlayer
#include "Engine/Engine.h"

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
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	// 무기 메시 — 캐릭터 오른손 본에 부착
	EquippedWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EquippedWeaponMesh"));
	EquippedWeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));   // 오른손 본
	EquippedWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedWeaponMesh->SetVisibility(false);      // 초기엔 안 보임 -> 장착 시 켜짐
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (PlayerInputConfig && PlayerInputConfig->InputMappingContext)
			{
				Subsystem->AddMappingContext(PlayerInputConfig->InputMappingContext, 0);
			}
		}
	}
	
	// 시작할 때 기본 무기 = 저격
	TakeRipleGun();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputConfig) return;

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (PlayerInputConfig->MoveAction)
			EIC->BindAction(PlayerInputConfig->MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::InputActionMove);
		if (PlayerInputConfig->LookAction)
			EIC->BindAction(PlayerInputConfig->LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::InputActionLook);
		if (PlayerInputConfig->JumpAction)
			EIC->BindAction(PlayerInputConfig->JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::InputActionJump);
		if (PlayerInputConfig->CrouchAction)
			EIC->BindAction(PlayerInputConfig->CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::InputActionCrouch);
		if (PlayerInputConfig->CrouchAction) 
			EIC->BindAction(PlayerInputConfig->CrouchAction, ETriggerEvent::Completed,    this, &APlayerCharacter::InputActionUnCrouch);
		if (PlayerInputConfig->SprintAction) 
			EIC->BindAction(PlayerInputConfig->SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
		if (PlayerInputConfig->SprintAction) 
			EIC->BindAction(PlayerInputConfig->SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
		if (PlayerInputConfig->RipleAction)
			EIC->BindAction(PlayerInputConfig->RipleAction,  ETriggerEvent::Started, this, &APlayerCharacter::InputActionRiple);
		if (PlayerInputConfig->FireAction)
			EIC->BindAction(PlayerInputConfig->FireAction, ETriggerEvent::Started, this, &APlayerCharacter::InputActionFire);

	}
}

void APlayerCharacter::UpdateMoveSpeed()
{
	if (bIsHit)
	{
		GetCharacterMovement()->MaxWalkSpeed = HitSpeed;
	}
	else if (bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;
	}
	else if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void APlayerCharacter::InputActionMove(const FInputActionValue& Value)
{
	const FVector2D MoveVec = Value.Get<FVector2D>();
	if (Controller == nullptr)
	{
		return;
	}

	// 카메라 yaw 방향 기준으로 forward/right 계산
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);  // pitch/roll 0, yaw만

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MoveVec.Y);   // W/S → 앞뒤
	AddMovementInput(RightDirection, MoveVec.X);   // A/D → 좌우
}

void APlayerCharacter::InputActionLook(const FInputActionValue& Value)
{
	const FVector2D LookVec = Value.Get<FVector2D>();
	AddControllerYawInput(LookVec.X);
	AddControllerPitchInput(LookVec.Y);
}

void APlayerCharacter::InputActionJump(const FInputActionValue& Value)
{
	Jump();   // ACharacter가 기본 제공하는 점프 함수
}

void APlayerCharacter::InputActionCrouch(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void APlayerCharacter::InputActionUnCrouch(const FInputActionValue& Value)
{
	UnCrouch();
}

void APlayerCharacter::StartSprint()
{
	bIsSprinting = true;
	UpdateMoveSpeed();
}

void APlayerCharacter::StopSprint()
{
	bIsSprinting = false;
	UpdateMoveSpeed();
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

void APlayerCharacter::OnHit()
{
	bIsHit = true;
	UpdateMoveSpeed();
}

void APlayerCharacter::EndHit()
{
	bIsHit = false;

	if (bIsCrouched)
	{
		// 앉은 상태라면 Crouched 속도를 사용
	}
	else
	{
		UpdateMoveSpeed();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Recover"));
}

void APlayerCharacter::TakeRipleGun()
{
	EquipWeapon(RipleWeaponData);
}

void APlayerCharacter::EquipWeapon(class UWeaponDataAsset* WeaponData)
{
	if (!WeaponData) return;

	CurrentWeapon = WeaponData->WeaponType;
    
	if (WeaponData->Mesh)
	{
		EquippedWeaponMesh->SetSkeletalMesh(WeaponData->Mesh);
	}
	EquippedWeaponMesh->SetRelativeTransform(WeaponData->AttachTransform);
	EquippedWeaponMesh->SetVisibility(true);
}

void APlayerCharacter::InputActionRiple(const FInputActionValue& Value)  { TakeRipleGun(); }

void APlayerCharacter::FireGun()
{
	if (CurrentWeapon != EWeaponType::RipleGun) return;
	if (!RipleWeaponData) return;

	// 카메라 위치에서 전방 5000까지 라인 트레이스
	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End   = Start + FirstPersonCamera->GetForwardVector() * 5000.0f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);   // 자기 자신은 무시

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);
	
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.5f, 0, 1.0f);

	if (bHit && Hit.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("FIRE HIT: %s | Damage: %d"),
			*Hit.GetActor()->GetName(), RipleWeaponData->Damage);

		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 15.0f, 12, FColor::Yellow, false, 0.5f);
	}
}

void APlayerCharacter::InputActionFire(const FInputActionValue& Value)
{
	FireGun();
}