// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ETDTDCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ETDEnemy.h"
#include "TimerManager.h"
#include <Containers/Array.h>
#include "EngineUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"

AETDTDCharacter::AETDTDCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(false); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = true; // Camera does not rotate relative to arm

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	//Dynamic Camera
		
	maxDist = 6000.0f;
	minDist = 1000.0f;

	zoomVelocity = 0.001f;
	maxArmLenght = 2000.0f;
	minArmLenght = 200.0f;

	maxDistanceEnemy = 0.0f;
	minDistanceEnemy = 10000.0f;

	alpha = 0.0f;

	socketOffsetZmax = 1.0f;
	zInicial = 600.0f;

	//input

	baseTurnRate = 45.0f;
	baseLookUpAtRate = 45.0f;
}


TArray<AETDEnemy*> AETDTDCharacter::GetAllActorsOfClass(TSubclassOf<AETDEnemy> ActorClass)
{
	TArray<AETDEnemy*> Actors;
	for (TActorIterator<AETDEnemy> ActorIterator(GetWorld(), ActorClass); ActorIterator; ++ActorIterator)
	{
		AETDEnemy* Actor = *ActorIterator;
		Actors.Add(Actor);
	}

	return Actors;
}


bool AETDTDCharacter::IsInViewport(AActor* TargetActor)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (!PlayerController)
	{
		return true;
	}

	FVector2D ScreenLocation;
	PlayerController->ProjectWorldLocationToScreen(TargetActor->GetActorLocation(), ScreenLocation);

	FVector2D ViewportSize;
	GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);

	return ScreenLocation.X > 0 && ScreenLocation.Y > 0 && ScreenLocation.X < ViewportSize.X && ScreenLocation.Y < ViewportSize.Y;
}

float AETDTDCharacter::GetDistanceFromCharacter(AActor* OtherActor)
{
	return this->GetDistanceTo(OtherActor);
}



void AETDTDCharacter::DynamicZoomEnemies()
{
	maxDistanceEnemy = minDist;
	minDistanceEnemy = maxDist;
	

	FVector forwardCharDir;
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRot(0, Rotation.Yaw, 0);
		forwardCharDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	}
	
	TArray<AETDEnemy*> enemigos = GetAllActorsOfClass(TargetableActors);
	int num = enemigos.Num();
	GEngine->AddOnScreenDebugMessage(8, 1.f, FColor::Emerald, FString::FromInt(num));

	for (auto& enemigo : enemigos)
	{
		FVector DirToEnemi = this->GetActorLocation() + (enemigo->GetActorLocation() - this->GetActorLocation());
		float dotp = FVector::DotProduct(DirToEnemi, forwardCharDir);
		if (IsInViewport(enemigo) && FVector::DotProduct(DirToEnemi, forwardCharDir) > 0)
		{
			GEngine->AddOnScreenDebugMessage(7, 0.5f, FColor::Yellow, FString::FromInt(dotp), false);
			float distance = GetDistanceFromCharacter(enemigo);
			GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::FromInt(distance));
			if (distance > maxDistanceEnemy)
			{
				maxDistanceEnemy = distance;
			}
			else if (distance < minDistanceEnemy)
			{
				minDistanceEnemy = distance;
			}
		}
	}

	if (maxDistanceEnemy > maxDist)
	{
		targetLenght = maxArmLenght;
	}
	else
	{
		targetLenght = minArmLenght + (maxArmLenght - minArmLenght) * (maxDistanceEnemy / maxDist);
	}
	
	float TargetsocketOffsetZ = socketOffsetZmax * (targetLenght - minArmLenght) / (maxArmLenght - minArmLenght);
	newSocketZ = zInicial + TargetsocketOffsetZ;

	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Yellow, FString::FromInt(newSocketZ));
	GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Red, FString::FromInt(maxDistanceEnemy));
	GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Red, FString::FromInt(minDistanceEnemy));

}

void AETDTDCharacter::MoveForward(float Value)
{
	if ((Controller) && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRot(0, Rotation.Yaw, 0);
		const FVector Dir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		AddMovementInput(Dir, Value);
	}
}

void AETDTDCharacter::MoveRight(float Value)
{
	if ((Controller) && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRot(0, Rotation.Yaw, 0);
		const FVector Dir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		AddMovementInput(Dir, Value);
	}
}

void AETDTDCharacter::TurnAtRate(float Value)
{
	AddControllerYawInput(Value * baseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AETDTDCharacter::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * baseLookUpAtRate * GetWorld()->GetDeltaSeconds());
}


void AETDTDCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}
		
	//DynamicZoomEnemies();
	float& springArmLenght = CameraBoom->TargetArmLength;
	float& CameraZOffset = CameraBoom->SocketOffset.Z;
	if (alpha < 1.0f)
	{
		alpha += (DeltaSeconds * zoomVelocity);
		springArmLenght = FMath::Lerp(springArmLenght, targetLenght, alpha);
		CameraZOffset = FMath::Lerp(CameraZOffset, newSocketZ, alpha); 
	}
	else
	{
		alpha = 0.0f;
	}

	FMath::Clamp(springArmLenght, minArmLenght, maxArmLenght);
	FMath::Clamp(CameraZOffset, 600.0f, 1500.0f);
	GEngine->AddOnScreenDebugMessage(4, 1.f, FColor::Green, FString::FromInt(springArmLenght));
		
}

void AETDTDCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(MemberTimerHandle, this, &AETDTDCharacter::DynamicZoomEnemies, 0.1f, true, -1.0f);
	
}

void AETDTDCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AETDTDCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AETDTDCharacter::MoveRight);

	PlayerInputComponent->BindAxis("TurnAtRate", this, &AETDTDCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpAtRate", this, &AETDTDCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}
