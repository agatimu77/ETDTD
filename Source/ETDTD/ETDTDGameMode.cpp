// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ETDTDGameMode.h"
#include "ETDTDPlayerController.h"
#include "ETDTDCharacter.h"
#include "UObject/ConstructorHelpers.h"

AETDTDGameMode::AETDTDGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AETDTDPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}