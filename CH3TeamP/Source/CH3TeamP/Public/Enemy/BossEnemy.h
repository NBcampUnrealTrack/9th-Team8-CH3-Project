#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy.h"
#include "BossEnemy.generated.h"

class ABossThrowProjectile;

UENUM(BlueprintType)
enum class EBossAttackType : uint8
{
	None UMETA(DisplayName = "None"),
	Melee UMETA(DisplayName = "Melee"),
	Area UMETA(DisplayName = "Area"),
	Throw UMETA(DisplayName = "Throw")
};

UCLASS()
class CH3TEAMP_API ABossEnemy : public ABaseEnemy
{
	GENERATED_BODY()
	
public:
	virtual void AttackTarget(AActor* TargetActor) override;
	
	UFUNCTION(BlueprintPure, Category="Boss|Attack")
	EBossAttackType GetBossAttackType() const;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Area Attack")
	float AreaAttackRange = 700.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Area Attack")
	float AreaAttackRadius = 250.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Area Attack")
	int32 AreaAttackDamage = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Area Attack")
	float AreaAttackCooldown = 6.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Area Attack")
	float AreaAttackWarningTime = 1.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area Attack")
	bool bCanAreaAttack = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Attack")
	EBossAttackType BossAttackType = EBossAttackType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Debug")
	bool bDrawAreaAttackDebug = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Throw")
	float ThrowSearchRadius = 1000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Throw")
	float ThrowCooldown = 8.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Throw")
	FName ThrowSocketName = TEXT("hand_rSocket");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boos|Throw")
	TSubclassOf<ABossThrowProjectile> ThrowProjectileClass;
	
private:
	FVector CachedAreaAttackLocation;
	
	FTimerHandle AreaAttackDelayTimerHandle;
	FTimerHandle AreaAttackCooldownTimerHandle;
	FTimerHandle ThrowCooldownTimerHandle;
	FTimerHandle BossAttackTypeResetTimerHandle;
	
	void StartAreaAttack(AActor* TargetActor);
	void ExecuteAreaAttack();
	void ResetAreaAttackCooldown();
	
	bool bCanThrowZombie = true;
	
	AActor* FindThrowableZombie() const;
	
	bool TryThrowZombie(AActor* TargetActor);
	
	void ResetThrowCooldown();
	
	void ResetBossAttackType();
};
