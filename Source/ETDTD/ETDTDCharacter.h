// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ETDTDCharacter.generated.h"


class AETDEnemy;


UCLASS(Blueprintable)
class AETDTDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AETDTDCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;


protected:

	//input
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	   
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float baseTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float baseLookUpAtRate;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Dynamic Camera

	struct FTimerHandle MemberTimerHandle;
	
	TArray<AETDEnemy*> GetAllActorsOfClass(TSubclassOf<AETDEnemy> Enemy);
		
	bool IsInViewport(AActor* TargetActor);

	float GetDistanceFromCharacter(AActor* OtherActor);

	void DynamicZoomEnemies();
			
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target System")
	TSubclassOf<AETDEnemy> TargetableActors;
	
	UPROPERTY(EditAnywhere, Category = "zoom")
	float zoomVelocity;

	UPROPERTY(EditAnywhere, Category = "zoom")
	float maxDist;

	UPROPERTY(EditAnywhere, Category = "zoom")
	float minDist;

	UPROPERTY(EditAnywhere, Category = "zoom")
	float maxArmLenght;

	UPROPERTY(EditAnywhere, Category = "zoom")
	float minArmLenght;
		
	float targetLenght;

	float maxDistanceEnemy;
	float minDistanceEnemy;

	float alpha;

	UPROPERTY(EditAnywhere, Category = "zoom")
	float socketOffsetZmax;
	float newSocketZ;
	float zInicial;


};

