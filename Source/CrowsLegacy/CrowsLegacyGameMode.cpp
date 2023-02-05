// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrowsLegacyGameMode.h"
#include "CrowsLegacyCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACrowsLegacyGameMode::ACrowsLegacyGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
