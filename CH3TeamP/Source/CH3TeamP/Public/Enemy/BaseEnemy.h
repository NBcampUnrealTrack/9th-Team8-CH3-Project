#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CH3GameplayTypes.h" // 추가: EEnemyType 사용
#include "BaseEnemy.generated.h"

class UHealthComponent;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Chase,
	Attack,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDiedSignature, ABaseEnemy*, DeadEnemy);

UCLASS()
class CH3TEAMP_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseEnemy();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void TakeEnemyDamage(int32 DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void Attack();

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void AttackTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetEnemyState(EEnemyState NewState);

	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyState GetEnemyState() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetAttackRange() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetAttackCooldown() const;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Event")
	FOnEnemyDiedSignature OnEnemyDied;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Component")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float Defense = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 AttackDamage = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float AttackCooldown = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 ExpReward = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyState EnemyState = EEnemyState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|HitSlow")
	bool bUseHitSlow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|HitSlow")
	float HitSlowMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|HitSlow")
	float HitSlowDuration = 0.3f;

	// 추가: 미니맵 갱신 / 킬카운트 / 킬피드 처치 통보용
	// BP_NormalEnemy, BP_RushEnemy, BP_TankerEnemy, BP_BossEnemy에서 각각 타입을 바꿔주면 됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	EEnemyType MyEnemyType = EEnemyType::Normal;

	FTimerHandle HitSlowTimerHandle;

protected:
	void ApplyHitSlow();
	void ResetMoveSpeed();
};