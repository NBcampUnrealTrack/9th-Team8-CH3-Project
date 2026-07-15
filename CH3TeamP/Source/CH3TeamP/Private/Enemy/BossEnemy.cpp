#include "Enemy/BossEnemy.h"
#include "Components/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void ABossEnemy::AttackTarget(AActor* TargetActor)
{
	// if (!TargetActor || bIsDead) 이렇게 했을 때 오류가 나는 이유는 bIsDead는 ABossEnemy의 멤버 변수가 아니고,
	// HealthComponent 안의 변수라서 오류가 남
	// 단, 이렇게 하려면 BaseEnemy.h에서 HealthComponent가 protected나 public이어야 함
	// 만약 private이면 BossEnemy에서 접근을 못 해서 또 에러가 남
	if (!TargetActor || HealthComponent || HealthComponent->bIsDead)
	{
		return;
	}
	
	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	
	if (bCanAreaAttack && DistanceToTarget <= AreaAttackRange)
	{
		StartAreaAttack(TargetActor);
		return;
	}
	
	Super::AttackTarget(TargetActor);
}

void ABossEnemy::StartAreaAttack(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}
	
	bCanAreaAttack = false;
	SetEnemyState(EEnemyState::Attack);
	
	CachedAreaAttackLocation = TargetActor->GetActorLocation();
	
	if (bDrawAreaAttackDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			CachedAreaAttackLocation,
			32,
			false,
			FColor::Red,
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
}

void ABossEnemy::ExecuteAreaAttack()
{
	TArray<AActor*> OverlappedActors;
	
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
		if (!Actor || Actor == this)
		{
			continue;
		}
		
		UHealthComponent* HealthComponent = Actor->FindComponentByClass<UHealthComponent>();
		if (HealthComponent)
		{
			HealthComponent->ApplyDamage(AreaAttackDamage);
		}
	}
}

void ABossEnemy::ResetAreaAttackCooldown()
{
	bCanAreaAttack = true;
}