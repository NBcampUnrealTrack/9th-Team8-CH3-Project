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
#include "CH3TeamProjectGameMode.h"
#include "Enemy/BaseEnemy.h"
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

void APlayerCharacter::AddEXP(float EXPValue)
{
	CurrentEXP += EXPValue;

	// 1. 단순 레벨업 체크 (while 대신 if로 1단계씩 처리)
	if (CurrentEXP >= MaxEXP)
	{
		CurrentEXP -= MaxEXP;
		CurrentLevel++;
		MaxEXP *= 1.2f; // 다음 레벨 필요 경험치 증가

		// 2. 레벨업 시 줌 해제 및 연사 중단 처리 (필요시 기존 구현 함수 호출)
		if (bIsAiming)
		{
			ToggleAim(); 
		}
		
		StopFire(); // 사격 중단 함수가 있다면 호출

		// 3. 레벨업 사운드 재생 (일시정지 되기 전 실행)
		if (LevelUpSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), LevelUpSound);
		}

		// ★ [UI 연동] 레벨업 이벤트 방송 (UI 담당자가 바인딩하여 레벨 텍스트 갱신)
		OnLevelUp.Broadcast(CurrentLevel);

		// 4. GameMode에 레벨업 알림 (게임 일시정지 및 카드 UI 호출)
		ACH3TeamProjectGameMode* GM = Cast<ACH3TeamProjectGameMode>(UGameplayStatics::GetGameMode(this));
		if (GM)
		{
			GM->NotifyPlayerLevelUp(GetController(), CurrentLevel);
		}
	}

	// ★ [UI 연동] 경험치 변경 이벤트 방송 (UI 담당자가 바인딩하여 EXP 프로그래스 바 갱신)
	OnEXPChanged.Broadcast(CurrentEXP, MaxEXP);
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
	
	// GameMode에 델리게이트 바인딩
	ACH3TeamProjectGameMode* GameMode = Cast<ACH3TeamProjectGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		// Dynamic Multicast Delegate이므로 AddDynamic을 사용합니다.
		GameMode->OnUpgradeConfirmed.AddDynamic(this, &APlayerCharacter::ApplyUpgrade);
	}
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
		if (PlayerInputConfig->TestDamageAction)
			EIC->BindAction(PlayerInputConfig->TestDamageAction, ETriggerEvent::Started, this, &APlayerCharacter::InputActionTestDamage);
		if (PlayerInputConfig->TestDeathAction)
			EIC->BindAction(PlayerInputConfig->TestDeathAction, ETriggerEvent::Started, this, &APlayerCharacter::InputActionTestDeath);
		
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

void  APlayerCharacter::InputActionFire(const FInputActionValue& Value)
{
}

void APlayerCharacter::FireGun()
{
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
	
	OnFireAnimation();   // 발사 몽타주
	
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), FireSound);
	}
	
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

void APlayerCharacter::PlayReloadMontage()
{
	if (ReloadMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(ReloadMontage);
	}
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
	
	// ★ 줌 상태라면 줌 해제
	if (bIsAiming)
	{
		ToggleAim(); // 이미 줌 중이므로 ToggleAim을 부르면 bIsAiming = false 및 TargetFOV = DefaultFOV 설정됨
	}
	
	bIsReloading = true;
	
	PlayReloadMontage();
	
	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&APlayerCharacter::FinishReload,
		ReloadTime,
		false);
}

void APlayerCharacter::FinishReload()
{
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	
	CurrentAmmoCount = MaxAmmo;
	bIsReloading = false;

	UE_LOG(LogTemp, Warning, TEXT("Reload Finish"));
}

void APlayerCharacter::OnDamage(int32 Amount)
{
	if (!HealthComp || HealthComp->bIsDead)
	{
		return;
	}
	
	HealthComp->ApplyDamage(Amount);

	if (HealthComp->bIsDead)
	{
		OnDeathAnimation();

		if (ACH3TeamProjectGameMode* GM =
			Cast<ACH3TeamProjectGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->NotifyPlayerDied(GetController());
		}
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
	if (bIsDead)
		return;

	bIsDead = true;
	
	if (ACH3TeamProjectGameMode* GM = Cast<ACH3TeamProjectGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->NotifyPlayerDied(GetController());
	}

	// 사망 애니메이션
	if (UPlayerAnimInstance* A = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		A->PlayDeathMontage(TEXT("DeathStart"));
	}

	// 입력 차단
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// 이동 중지
	GetCharacterMovement()->DisableMovement();

	// 충돌 제거
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3초 후 제거
	SetLifeSpan(3.f);
}

void APlayerCharacter::InputActionTestDamage(const FInputActionValue& Value)
{
	OnDamage(30);
}

void APlayerCharacter::InputActionTestDeath(const FInputActionValue& Value)
{
	OnDamage(9999);
}


void APlayerCharacter::FireNormal()
{
	
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
		
		// 맞은 액터가 몬스터인지 확인
		if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(Hit.GetActor()))
		{
			Enemy->TakeEnemyDamage((int32)Damage);
			OnDamageDealt.Broadcast(Damage, Hit.ImpactPoint, false);

		}
		
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
		ECC_GameTraceChannel1,
		Params);

	if (!bHit)
		return;

	float Damage = GetCurrentDamage();

	for (const FHitResult& Hit : Hits)
	{
		// 맞은 위치에는 무조건 이펙트 생성
		if (ImpactEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactEffect,
				Hit.ImpactPoint);
		}

		AActor* HitActor = Hit.GetActor();

		ABaseEnemy* Enemy = Cast<ABaseEnemy>(HitActor);

		if (Enemy)
		{
			Enemy->TakeEnemyDamage((int32)Damage);
			OnDamageDealt.Broadcast(Damage, Hit.ImpactPoint, false);

		}

		if (Hit.bBlockingHit && !Enemy)
		{
			break;
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
	if (ExplosionImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionImpactEffect,
			Hit.ImpactPoint);
	}
	
	FTimerHandle Timer;

	GetWorldTimerManager().SetTimer(
		Timer,
		[this, Hit]()
		{
			if (ExplosionSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					ExplosionSound,
					Hit.ImpactPoint);
			}
		},
		0.3f,
		false);

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

	float Damage = GetCurrentDamage();
	
	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();

		if (!HitActor)
			continue;

		if (HitActor == this)
			continue;
		
		if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(HitActor))
		{
			Enemy->TakeEnemyDamage((int32)Damage);
			OnDamageDealt.Broadcast(Damage, HitActor->GetActorLocation() + FVector(0.f, 0.f, 50.f), false);


		}
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
}

void APlayerCharacter::ApplyUpgrade(EUpgradeType ChosenUpgrade, AController* ForPlayer)
{
	// 나를 조종하는 컨트롤러에게 전달된 이벤트인지 안전 검사
	if (ForPlayer && ForPlayer != GetController())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("PlayerCharacter: 강화 적용 -> %d"), static_cast<int32>(ChosenUpgrade));

	switch (ChosenUpgrade)
	{
	case EUpgradeType::AttackUp:
		// 모든 탄종의 데미지를 일괄 20% 증가
		NormalDamage *= 1.2f;
		PiercingDamage *= 1.2f;
		ExplosiveDamage *= 1.2f;
        
		UE_LOG(LogTemp, Log, TEXT("공격력 강화 완료! Normal: %.1f, Piercing: %.1f, Explosive: %.1f"), 
			   NormalDamage, PiercingDamage, ExplosiveDamage);
		break;
		
	case EUpgradeType::FireRateUp:

		FireRate *= 1.2f;

		if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
		{
			StopFire();
			StartFire();
		}

		break;

	case EUpgradeType::MoveSpeedUp:
		// [이동 속도 10% 증가] 기본 걷기 속도 및 스프린트(달리기) 속도 1.10배 상향
		WalkSpeed *= 1.10f;
		SprintSpeed *= 1.10f;
		UpdateMoveSpeed();
		break;

	case EUpgradeType::MagazineUp:
		// [탄창 용량 고정 +10 증가] 최대 탄창 수 10발 추가 및 탄약 즉시 완충
		MaxAmmo += 10;
		CurrentAmmoCount = MaxAmmo; 
		break;

	case EUpgradeType::ExplosiveAmmo:
		// [폭발탄 전환] 현재 탄종을 폭발탄(Explosive)으로 변경
		CurrentAmmoType = EAmmoType::Explosive;
		
		break;

	case EUpgradeType::PiercingAmmo:
		// [관통탄 전환] 현재 탄종을 관통탄(Piercing)으로 변경
		CurrentAmmoType = EAmmoType::Piercing;
		
		break;

	case EUpgradeType::StaminaUp:
		// [최대 스태미나 20% 증가 및 현재 스태미나 채움]
		if (StaminaComp)
		{
			StaminaComp->IncreaseMaxStamina(20.f);
			
			UE_LOG(LogTemp, Log, TEXT("스태미나 +20 증가 완료! (현재 최대 스태미나: %.1f)"), StaminaComp->GetMaxStamina());
		}
		break;

	case EUpgradeType::Heal:
		// [체력 완전 회복 (풀피)] CurrentHP를 MaxHp 수치로 직접 설정
		if (HealthComp)
		{
			HealthComp->CurrentHP = HealthComp->MaxHp;

			UE_LOG(LogTemp, Log, TEXT("체력 완전 회복 완료! (현재 HP: %d / %d)"), HealthComp->CurrentHP, HealthComp->MaxHp);
		}
		break;

	default:
		break;
	}
}