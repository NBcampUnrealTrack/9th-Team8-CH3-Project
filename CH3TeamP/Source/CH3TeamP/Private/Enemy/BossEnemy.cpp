#include "Enemy/BossEnemy.h"
#include "Enemy/BossThrowProjectile.h"
#include "Components/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

class ABossThrowProjectile;

void ABossEnemy::AttackTarget(AActor* TargetActor)
{
	// if (!TargetActor || bIsDead) 이렇게 했을 때 오류가 나는 이유는 bIsDead는 ABossEnemy의 멤버 변수가 아니고,
	// HealthComponent 안의 변수라서 오류가 남
	// 단, 이렇게 하려면 BaseEnemy.h에서 HealthComponent가 protected나 public이어야 함
	// 만약 private이면 BossEnemy에서 접근을 못 해서 또 에러가 남
	if (!IsValid(TargetActor) || !HealthComponent || HealthComponent->bIsDead)
	{
		return;
	}
	
	const float DistanceToTarget = FVector::Dist(
		GetActorLocation(), 
		TargetActor->GetActorLocation()
		);
	
	if (bCanThrowZombie && TryThrowZombie(TargetActor))
	{
		return;
	}
	
	if (bCanAreaAttack && DistanceToTarget <= AreaAttackRange)
	{
		StartAreaAttack(TargetActor);
		return;
	}
	
	BossAttackType = EBossAttackType::Melee;
	
	Super::AttackTarget(TargetActor);
	
	GetWorldTimerManager().SetTimer(
		BossAttackTypeResetTimerHandle,
		this,
		&ABossEnemy::ResetBossAttackType,
		1.0f,
		false
		);
}

EBossAttackType ABossEnemy::GetBossAttackType() const
{
	return BossAttackType;
}

void ABossEnemy::StartAreaAttack(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return;
	}
	
	SetEnemyState(EEnemyState::Attack);
	BossAttackType = EBossAttackType::Area;
	bCanAreaAttack = false;
	
	CachedAreaAttackLocation = TargetActor->GetActorLocation();
	
	if (bDrawAreaAttackDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			CachedAreaAttackLocation,
			AreaAttackRadius,
			32,
			FColor::Red,
			false,
			AreaAttackWarningTime,
			0,
			3.0f
			);
	}
	
	GetWorldTimerManager().SetTimer(
		AreaAttackDelayTimerHandle,
		this,
		&ABossEnemy::ExecuteAreaAttack,
		AreaAttackWarningTime,
		false
		);
	
	GetWorldTimerManager().SetTimer(
		AreaAttackCooldownTimerHandle,
		this,
		&ABossEnemy::ResetAreaAttackCooldown,
		AreaAttackCooldown,
		false
		);
	
	GetWorldTimerManager().SetTimer(
		BossAttackTypeResetTimerHandle,
		this,
		&ABossEnemy::ResetBossAttackType,
		AreaAttackWarningTime,
		false
		);
}

void ABossEnemy::ExecuteAreaAttack()
{
	TArray<AActor*> OverlappedActors;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		CachedAreaAttackLocation,
		AreaAttackRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		nullptr,
		TArray<AActor*>(),
		OverlappedActors
		);
	
	for (AActor* Actor : OverlappedActors)
	{
		if (!IsValid(Actor) || Actor == this)
		{
			continue;
		}
		
		UHealthComponent* TargetHealthComponent = Actor->FindComponentByClass<UHealthComponent>();
		if (TargetHealthComponent)
		{
			TargetHealthComponent->ApplyDamage(AreaAttackDamage);
		}
	}
}

void ABossEnemy::ResetAreaAttackCooldown()
{
	bCanAreaAttack = true;
}

AActor* ABossEnemy::FindThrowableZombie() const
{
	TArray<AActor*> OverlappedActors;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		ThrowSearchRadius,
		ObjectTypes,
		nullptr,
		TArray<AActor*>(),
		OverlappedActors
		);
	
	for (AActor* Actor : OverlappedActors)
	{
		if (!IsValid(Actor) || Actor == this)
		{
			continue;
		}
		
		if (Actor->ActorHasTag(TEXT("ThrowableZombie")))
		{
			return Actor;
		}
	}
	
	return nullptr;
}

bool ABossEnemy::TryThrowZombie(AActor* TargetActor)
{
	if (!bCanThrowZombie || !IsValid(TargetActor) || !ThrowProjectileClass)
	{
		return false;
	}
	
	AActor* ThrowableZombie = FindThrowableZombie();
	if (!IsValid(ThrowableZombie))
	{
		return false;
	}
	
	bCanThrowZombie = false;
	
	SetEnemyState(EEnemyState::Attack);
	BossAttackType = EBossAttackType::Throw;
	
	ThrowableZombie->Destroy();
	
	const FVector SpawnLocation = GetMesh()->DoesSocketExist(ThrowSocketName)
	? GetMesh()->GetSocketLocation(ThrowSocketName)
	: GetActorLocation() + GetActorForwardVector() * 150.0f + FVector(0.0f, 0.0f,120.0f);
	
	const FVector ThrowTargetLocation = TargetActor->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	
	const FVector ThrowDirection = (ThrowTargetLocation - SpawnLocation).GetSafeNormal();
	const FRotator SpawnRotation = ThrowDirection.Rotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ABossThrowProjectile* Projectile = GetWorld()->SpawnActor<ABossThrowProjectile>(
		ThrowProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
		);
	
	if (Projectile)
	{
		Projectile->FireInDirection(ThrowDirection);
	}
	
	GetWorldTimerManager().SetTimer(
		ThrowCooldownTimerHandle,
		this,
		&ABossEnemy::ResetThrowCooldown,
		ThrowCooldown,
		false
		);
	
	GetWorldTimerManager().SetTimer(
		BossAttackTypeResetTimerHandle,
		this,
		&ABossEnemy::ResetBossAttackType,
		1.0f,
		false
		);
	
	return true;
}

void ABossEnemy::ResetThrowCooldown()
{
	bCanThrowZombie = true;
}

void ABossEnemy::ResetBossAttackType()
{
	BossAttackType = EBossAttackType::None;
}