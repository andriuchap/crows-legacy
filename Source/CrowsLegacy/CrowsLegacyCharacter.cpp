// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrowsLegacyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"


//////////////////////////////////////////////////////////////////////////
// ACrowsLegacyCharacter

ACrowsLegacyCharacter::ACrowsLegacyCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 700.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);

	bWantsToGlide = false;
}

void ACrowsLegacyCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACrowsLegacyCharacter::CheckJumpInput(float DeltaTime)
{
	Super::CheckJumpInput(DeltaTime);

	if (GetCharacterMovement()->IsFalling() && GetCharacterMovement()->Velocity.Z <= 0.0F)
	{
		if (bWantsToGlide)
		{
			GetCharacterMovement()->GravityScale = 0.25F;
		}
	}
}

void ACrowsLegacyCharacter::Jump()
{
	Super::Jump();
	UE_LOG(LogTemp, Warning, TEXT("Wants to glide"));
	bWantsToGlide = true;
}

void ACrowsLegacyCharacter::StopJumping()
{
	Super::StopJumping();
	UE_LOG(LogTemp, Warning, TEXT("No more glide"));
	GetCharacterMovement()->GravityScale = 1.75F;
	bWantsToGlide = false;
}

void ACrowsLegacyCharacter::Dig()
{
	if (DigMontage != nullptr)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DigMontage);
		GetCharacterMovement()->SetMovementMode(MOVE_None);
		bShouldReturnToDefaultMoveMode = true;
	}
}

void ACrowsLegacyCharacter::OnItemFound()
{
	bShouldReturnToDefaultMoveMode = false;
}

void ACrowsLegacyCharacter::DigFinished()
{
	if (bShouldReturnToDefaultMoveMode)
	{
		GetCharacterMovement()->SetDefaultMovementMode();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACrowsLegacyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACrowsLegacyCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACrowsLegacyCharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACrowsLegacyCharacter::Move);

		EnhancedInputComponent->BindAction(DigAction, ETriggerEvent::Triggered, this, &ACrowsLegacyCharacter::Dig);
	}

}

void ACrowsLegacyCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	FVector ForwardVector = FVector::VectorPlaneProject(FollowCamera->GetForwardVector(), FVector::UpVector).GetSafeNormal();

	AddMovementInput(ForwardVector, MovementVector.Y);
	AddMovementInput(FollowCamera->GetRightVector(), MovementVector.X);
}




