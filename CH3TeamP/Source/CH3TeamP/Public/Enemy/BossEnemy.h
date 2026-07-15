#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy.h"
#include "BossEnemy.generated.h"


UCLASS()
class CH3TEAMP_API ABossEnemy : public ABaseEnemy
{
	GENERATED_BODY()
	
public:
	virtual void AttackTarget(AActor* TargetActor) override;
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Debug")
	bool bDrawAreaAttackDebug = true;
	
private:
	bool bCanAreaAttack = true;
	
	FTimerHandle AreaAttackCooldownTimerHandle;
	FTimerHandle AreaAttackDelayTimerHandle;
	
	FVector CachedAreaAttackLocation;
	
	void StartAreaAttack(AActor* TargetActor);
	void ExecuteAreaAttack();
	void ResetAreaAttackCooldown();
	
};
