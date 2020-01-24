// Fill out your copyright notice in the Description page of Project Settings.


#include "ETDEnemy.h"

// Sets default values
AETDEnemy::AETDEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AETDEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AETDEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

