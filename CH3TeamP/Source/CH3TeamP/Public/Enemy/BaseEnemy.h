#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stats")
	float CurrentHealth = 100.f;
	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	bool bIsDead = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	EEnemyState EnemyState = EEnemyState::Idle;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void TakeEnemyDamage(float DamageAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual void Attack();
	
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
