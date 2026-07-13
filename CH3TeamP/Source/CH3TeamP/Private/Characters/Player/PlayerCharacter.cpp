#include "Characters/Player/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h" // UEnhancedInputComponent, BindAction()
#include "EnhancedInputSubsystems.h" // UEnhancedInputLocalPlayerSubsystem, AddMappingContext
#include "InputActionValue.h" // FInputActionValue
#include "Input/PlayerInputConfigDataAsset.h"
#include "Weapons/WeaponDataAsset.h"
#include "Components/HealthComponent.h"
#include "Components/StaminaComponent.h"
#include "NiagaraComponent.h"  // 머즐 컴포넌트용
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h" // 임팩트 스폰용
#include "GameFramework/PlayerController.h" // APlayerController
#include "Animation/PlayerAnimInstance.h"
#include "Engine/LocalPlayer.h" // ULocalPlayer
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->bUsePawnControlRotation = true;
	GetMesh()->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
	GetMesh()->SetOnlyOwnerSee(true);
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetCastShadow(false);
	
	ThirdPersonMesh =CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->SetupAttachment(GetCapsuleComponent());
	ThirdPersonMesh->SetRelativeLocation(GetMesh()->GetRelativeLocation());
	ThirdPersonMesh->SetLeaderPoseComponent(GetMesh());
	ThirdPersonMesh->SetOnlyOwnerSee(false);
	ThirdPersonMesh->SetOwnerNoSee(true);
	ThirdPersonMesh->SetCastShadow(true);
	

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
	
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	
	// 머즐: 무기 메시의 Muzzle 소켓에 부착 평소엔 꺼둠
	MuzzleComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MuzzleComp"));
	MuzzleComp->SetupAttachment(EquippedWeaponMesh, TEXT("Muzzle"));
	MuzzleComp->SetAutoActivate(false);
	
	StaminaComp = CreateDefaultSubobject<UCH3StaminaComponent>(TEXT("StaminaComp"));
	
	
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentFOV = DefaultFOV;
	TargetFOV = DefaultFOV;

	FirstPersonCamera->SetFieldOfView(DefaultFOV);
	
	ThirdPersonMesh->SetSkeletalMesh(GetMesh()->GetSkeletalMeshAsset());
	ThirdPersonMesh->SetAnimInstanceClass(GetMesh()->GetAnimClass());

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
	
	StandingCameraHeight = FirstPersonCamera->GetRelativeLocation().Z;
	TargetCameraHeight = StandingCameraHeight;
	// 원하는 만큼만 내리기
	CrouchCameraHeight = StandingCameraHeight - 32.f;
	
	
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	FVector CameraLocation = FirstPersonCamera->GetRelativeLocation();
	
	if (FMath::IsNearlyEqual(CameraLocation.Z, TargetCameraHeight, 0.1f))
	{
		CameraLocation.Z = TargetCameraHeight;
	}
	
	CurrentFOV = FMath::FInterpTo(
		CurrentFOV,
		TargetFOV,
		DeltaTime,
		AimInterpSpeed);
	

	FirstPersonCamera->SetFieldOfView(CurrentFOV);

	if (!StaminaComp)
	{
		return;
	}

	if (bIsSprinting)
	{
		// 움직일 때만 스태미나 감소
		if (GetVelocity().SizeSquared() > 1.f)
		{
			StaminaComp->ConsumeStamina(DeltaTime);

			if (!StaminaComp->CanSprint())
			{
				StopSprint();
			}
		}
	}
	else
	{
		// 달리지 않을 때 회복
		StaminaComp->RecoverStamina(DeltaTime);
	}
	
	
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
		{
			EIC->BindAction(PlayerInputConfig->FireAction,	ETriggerEvent::Started,this,  &APlayerCharacter::StartFire);
			EIC->BindAction(PlayerInputConfig->FireAction,	ETriggerEvent::Completed,this,  &APlayerCharacter::StopFire);
			EIC->BindAction(PlayerInputConfig->FireAction,	ETriggerEvent::Canceled,this,	 &APlayerCharacter::StopFire);
		}
		if (PlayerInputConfig->AimAction)
			EIC->BindAction(PlayerInputConfig->AimAction, ETriggerEvent::Started,this, &APlayerCharacter::ToggleAim);
		if (PlayerInputConfig->ReloadAction)
			EIC->BindAction(PlayerInputConfig->ReloadAction, ETriggerEvent::Started, this, &APlayerCharacter::Reload);
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

void APlayerCharacter::InputActionJump(const struct FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling())
	{
		return; // 이미 공중이면 무시
	}
	if (GetWorld()->TimeSince(LastLandedTime) < 0.2f)
	{
		return; // 착지 후 0.2초 잠금
	}
	Jump();
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	LastLandedTime = GetWorld()->GetTimeSeconds();   // 착지 시각 기록
}

void APlayerCharacter::InputActionCrouch(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		UnCrouch();
		TargetCameraHeight = StandingCameraHeight;
	}
	else
	{
		Crouch();
		TargetCameraHeight = CrouchCameraHeight;
	}
}

void APlayerCharacter::InputActionUnCrouch(const FInputActionValue& Value)
{
	UnCrouch();
}

void APlayerCharacter::StartSprint()
{
	if (bIsSprinting)
	{
		return;
	}
	
	if (bIsReloading)
	{
		return;
	}
	
	if (!StaminaComp)
	{
		return;
	}

	if (!StaminaComp->CanSprint())
	{
		return;
	}

	// 조준 중이었다면 해제
	if (bIsAiming)
	{
		bIsAiming = false;
		TargetFOV = DefaultFOV;
	}
	
	bIsSprinting = true;
	UpdateMoveSpeed();
}

void APlayerCharacter::StopSprint()
{
	if (!bIsSprinting)
	{
		return;
	}

	bIsSprinting = false;
	UpdateMoveSpeed();
}

void APlayerCharacter::ToggleAim()
{
	if (bIsSprinting)
	{
		return;
	}
	
	if (bIsReloading)
	{
		return;
	}

	bIsAiming = !bIsAiming;
	TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
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
	
	switch(CurrentAmmoType)
	{
	case EAmmoType::Normal:
		FireNormal();
		break;

	case EAmmoType::Piercing:
		FirePiercing();
		break;

	case EAmmoType::Explosive:
		FireExplosive();
		break;
	}
	
}

void APlayerCharacter::StartFire()
{
	// 이미 연사 중이면 무시
	if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		return;
	}

	// 첫 발은 즉시 발사
	FireGun();

	GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&APlayerCharacter::FireGun,
		1.f / FireRate,
		true
	);
}

void APlayerCharacter::StopFire()
{
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void APlayerCharacter::Reload(const FInputActionValue& Value)
{
	if (bIsReloading)
	{
		return;
	}

	if (CurrentAmmoCount == MaxAmmo)
	{
		return;
	}
	bIsReloading = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Reload Start"));

	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&APlayerCharacter::FinishReload,
		ReloadTime,
		false);
}

void APlayerCharacter::FinishReload()
{
	CurrentAmmoCount = MaxAmmo;
	bIsReloading = false;

	UE_LOG(LogTemp, Warning, TEXT("Reload Finish"));
}

void APlayerCharacter::InputActionFire(const FInputActionValue& Value)
{
}

void APlayerCharacter::OnDamage(int32 Amount)
{
	if (!HealthComp || HealthComp->bIsDead)
	{
		return;
	}
	
	HealthComp->ApplyDamage(Amount);

	if (HealthComp->CurrentHP <= 0)
	{
		OnDeathAnimation();
	}
	else
	{
		OnHitAnimation();
	}
}

void APlayerCharacter::OnFireAnimation()
{
	if (UPlayerAnimInstance* A = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
		A->PlayFireMontage();
}

void APlayerCharacter::OnHitAnimation()
{
	if (UPlayerAnimInstance* A = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		int32 Idx = FMath::RandRange(0, 1);
		A->PlayDamageMontage(FName(*FString::Printf(TEXT("HitReact%d"), Idx)));
	}
}

void APlayerCharacter::OnDeathAnimation()
{
	if (UPlayerAnimInstance* A = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
		A->PlayDeathMontage(TEXT("DeathStart"));
}

void APlayerCharacter::InputActionTestDamage(const struct FInputActionValue& Value)
{
	OnDamage(30);
}

void APlayerCharacter::InputActionTestDeath(const struct FInputActionValue& Value)
{
	OnDamage(9999);
}

void APlayerCharacter::FireNormal()
{
	OnFireAnimation();   // 발사 몽타주
	
	if (bIsReloading)
	{
		return;
	}

	if (CurrentAmmoCount <= 0)
	{
		Reload(FInputActionValue());
		return;
	}

	CurrentAmmoCount--;

	UE_LOG(LogTemp, Warning, TEXT("Ammo : %d"), CurrentAmmoCount); // 탄창 수 UI 나올경우 삭제 예정
	
	// (발사 직후) 머즐 플래시 - 한 번 터뜨리고, 루프 방지로 잠깐 뒤 끔
	if (MuzzleComp)
	{
		MuzzleComp->Activate(true);

		FTimerHandle MuzzleOffTimer;
		GetWorldTimerManager().SetTimer(MuzzleOffTimer, [this]()
		{
			if (MuzzleComp)
			{
				MuzzleComp->Deactivate();
			}
		}, 0.1f, false);
	}
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

	if (bHit && Hit.GetActor())
	{
		float Damage = GetCurrentDamage();
		
		// Monster->TakeDamage(Damage);
		// TODO 몬스터한테 피격 처리 구현 예정
		
		// 임팩트 이펙트 — 맞은 지점에 한 번 스폰 (Cascade)
		if (ImpactEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactEffect,
			Hit.ImpactPoint);
		}
		
	}
}

void APlayerCharacter::FirePiercing()
{
	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * 10000.f;

	TArray<FHitResult> Hits;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceMultiByChannel(
		Hits,
		Start,
		End,
		ECC_Visibility,
		Params);

	if (!bHit)
		return;

	for (const FHitResult& Hit : Hits)
	{
		if (!Hit.GetActor())
			continue;
		
		float Damage = GetCurrentDamage();

		// Monster->TakeDamage(Damage);
		// TODO 몬스터한테 피격 처리 구현 예정
		
		// 맞은 위치마다 이펙트
		if (ImpactEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactEffect,
				Hit.ImpactPoint);
		}
	}
}

void APlayerCharacter::FireExplosive()
{
	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * 10000.f;

	FHitResult Hit;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params);

	if (!bHit)
		return;

	// 폭발 이펙트
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactEffect,
			Hit.ImpactPoint);
	}

	float ExplosionRadius = 300.f;

	TArray<FOverlapResult> Overlaps;

	FCollisionShape Sphere =
		FCollisionShape::MakeSphere(ExplosionRadius);

	bool bOverlap = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Hit.ImpactPoint,
		FQuat::Identity,
		ECC_Pawn,
		Sphere);

	if (!bOverlap)
		return;

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();

		if (!HitActor)
			continue;

		if (HitActor == this)
			continue;

		float Damage = GetCurrentDamage();
		
		// TODO 몬스터한테 피격 처리 구현 예정
		// HitActor->TakeDamage(...);
	}
}

float APlayerCharacter::GetCurrentDamage() const
{
	switch (CurrentAmmoType)
	{
	case EAmmoType::Piercing:
		return PiercingDamage;

	case EAmmoType::Explosive:
		return ExplosiveDamage;

	case EAmmoType::Normal:
	default:
		return NormalDamage;
}
