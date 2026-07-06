#include "EnemyAIController.h"
#include "BaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	
	TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!TargetPlayer || !GetPawn())
	{
		return;
	}
	
	ABaseEnemy* ControlledEnemy = Cast<ABaseEnemy>(GetPawn());
	if (!ControlledEnemy)
	{
		return;
	}
	
	const float DistanceToPlayer = 
		FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation());
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			0.0f,
			FColor::Yellow,
			FString::Printf(TEXT("Distance: %.2f"), DistanceToPlayer)
			);
	}
	
	const float AttackRange = 300.0f;
	
	if (DistanceToPlayer > AttackRange)
	{
		MoveToActor(TargetPlayer, AttackRange);
	}
	else
	{
		StopMovement();
		ControlledEnemy->Attack();
	}
}

