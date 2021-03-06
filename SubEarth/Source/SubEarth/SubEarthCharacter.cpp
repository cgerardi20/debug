// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SubEarth.h"
#include "SubEarthCharacter.h"
#include "SubEarthProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ASubEarthCharacter

/******************************************************************************/
ASubEarthCharacter::ASubEarthCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);
	Mesh1P->SetHiddenInGame(true, true);

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->Hand = EControllerHand::Right;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Initialize member variables
	isLeftHandEmpty = true;
	isRightHandEmpty = true;
}

/******************************************************************************/
void ASubEarthCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("ASubEarthCharacter started"));

}

/******************************************************************************/
void ASubEarthCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ASubEarthCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Left_Hand_Grab_Drop", IE_Pressed, this, &ASubEarthCharacter::LeftHandGrabDropObj);
	PlayerInputComponent->BindAction("Right_Hand_Grab_Drop", IE_Pressed, this, &ASubEarthCharacter::RightHandGrabDropObj);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASubEarthCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASubEarthCharacter::MoveRight);



	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASubEarthCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASubEarthCharacter::LookUpAtRate);
	UE_LOG(LogTemp, Log, TEXT("Binding Complete"));
}

/******************************************************************************/
bool ASubEarthCharacter::IsHandEmpty(int hand)
{
	bool is_empty = false;

	switch ((Hand_e)hand)
	{
		case LEFT_HAND: is_empty = isLeftHandEmpty; break;
		case RIGHT_HAND: is_empty = isRightHandEmpty; break;
		case BOTH_HANDS: 
			if (isLeftHandEmpty && isRightHandEmpty)
			{
				is_empty = true;
			}
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Unrecognized hand enum %d"), hand);
			break;
	}

	return is_empty;
}

/******************************************************************************/
void ASubEarthCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
	UE_LOG(LogTemp, Log, TEXT("VR was reset"));
}

/******************************************************************************/
void ASubEarthCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

/******************************************************************************/
void ASubEarthCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

/******************************************************************************/
void ASubEarthCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

/******************************************************************************/
void ASubEarthCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

/******************************************************************************/
void ASubEarthCharacter::LeftHandGrabDropObj()
{
	GrabDropObject(LEFT_HAND);
}

/******************************************************************************/
void ASubEarthCharacter::RightHandGrabDropObj()
{
	GrabDropObject(RIGHT_HAND);
}

/******************************************************************************/
void ASubEarthCharacter::GrabDropObject(int hand)
{
	switch ((Hand_e)hand)
	{
		case LEFT_HAND:
		{
			if (isLeftHandEmpty)
			{
				isLeftHandEmpty = false;
				UE_LOG(LogTemp, Log, TEXT("Left hand grab object"));
			}
			else
			{
				isLeftHandEmpty = true;
				UE_LOG(LogTemp, Log, TEXT("Left hand drop object"));
			}
			break;
		}
		
		case RIGHT_HAND: 
		{
			if (isRightHandEmpty)
			{
				isRightHandEmpty = false;
				UE_LOG(LogTemp, Log, TEXT("Right hand grab object"));
			}
			else
			{
				isRightHandEmpty = true;
				UE_LOG(LogTemp, Log, TEXT("Right hand drop object"));
			}
			break;
		}
		case BOTH_HANDS:
		{
			GrabDropObject(LEFT_HAND);
			GrabDropObject(RIGHT_HAND);
			break;
		}

		default:
			UE_LOG(LogTemp, Warning, TEXT("Unrecognized hand enum %d"), hand);
			break;
	}
}

