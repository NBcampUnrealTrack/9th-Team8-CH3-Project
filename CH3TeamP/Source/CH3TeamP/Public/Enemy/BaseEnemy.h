#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

class UHealthComponent;
class AActor;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Chase,
	Attack,
	Dead
};

UCLASS()
class CH3TEAMP_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseEnemy();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Component")
	UHealthComponent* HealthComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float Defense = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float MoveSpeed = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackRange = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackCooldown = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	float DetectRange = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	int32 AttackDamage = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reward")
	int32 ExpReward = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reward")
	TSubclassOf<AActor> ExpPickupClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	EEnemyState EnemyState = EEnemyState::Idle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|HitSlow")
	bool bUseHitSlow = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|HitSlow")
	float HitSlowMultiplier = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|HitSlow")
	float HitSlowDuration = 0.3f;
	
	FTimerHandle HitSlowTimerHandle;
	
protected:
	void ApplyHitSlow();
	void ResetMoveSpeed();
	
public:
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void TakeEnemyDamage(int32 DamageAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void Attack();
	
	UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
	virtual void AttackTarget(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void Die();
	
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetEnemyState(EEnemyState NewState);
	
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	EEnemyState GetEnemyState() const;
	
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	float GetAttackRange() const;
	
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	float GetAttackCooldown() const;
	
};
