#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExpPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class CH3TEAMP_API AExpPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AExpPickup();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exp")
	USphereComponent* SphereCollision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exp")
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
	int32 ExpAmount = 10;

};
