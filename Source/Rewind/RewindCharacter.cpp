// Copyright Epic Games, Inc. All Rights Reserved.

#include "RewindCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "RewindComponent.h"
#include "RewindGameMode.h"
#include "RewindVisualizationComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ARewindCharacter

ARewindCharacter::ARewindCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;            // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;       // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false;                              

	// Setup a rewind component that snapshots 30Hz
	RewindComponent = CreateDefaultSubobject<URewindComponent>(TEXT("RewindComponent"));
	RewindComponent->SnapshotFrequencySeconds = 1.0f / 30.0f;	
	RewindComponent->bSnapshotMovementVelocityAndMode = true;
	RewindComponent->bPauseAnimationDuringTimeScrubbing = true;

	// Setup a rewind visualization component that draws a static mesh instance for each snapshot
	RewindVisualizationComponent = CreateDefaultSubobject<URewindVisualizationComponent>(TEXT("RewindVisualizationComponent"));
	RewindVisualizationComponent->SetupAttachment(RootComponent);
}

void ARewindCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Capture game mode for driving global rewind
	GameMode = Cast<ARewindGameMode>(GetWorld()->GetAuthGameMode());
	OriginalTargetCameraArmLength = CameraBoom->TargetArmLength;
	RewindComponent->OnTimeManipulationStarted.AddUniqueDynamic(this, &ARewindCharacter::UpdateCamera);
	RewindComponent->OnTimeManipulationCompleted.AddUniqueDynamic(this, &ARewindCharacter::UpdateCamera);
}

void ARewindCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARewindCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARewindCharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARewindCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARewindCharacter::Look);
		EnhancedInputComponent->BindAction(ToggleTimeScrubAction, ETriggerEvent::Started, this, &ARewindCharacter::ToggleTimeScrub);
		EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Started, this, &ARewindCharacter::Rewind);
		EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Completed, this, &ARewindCharacter::StopRewinding);
		EnhancedInputComponent->BindAction(FastForwardAction, ETriggerEvent::Started, this, &ARewindCharacter::FastForward);
		EnhancedInputComponent->BindAction(FastForwardAction, ETriggerEvent::Completed, this, &ARewindCharacter::StopFastForwarding);
		EnhancedInputComponent->BindAction(SetRewindSpeedSlowestAction, ETriggerEvent::Started, this, &ARewindCharacter::SetRewindSpeedSlowest);
		EnhancedInputComponent->BindAction(SetRewindSpeedSlowerAction, ETriggerEvent::Started, this, &ARewindCharacter::SetRewindSpeedSlower);
		EnhancedInputComponent->BindAction(SetRewindSpeedNormalAction, ETriggerEvent::Started, this, &ARewindCharacter::SetRewindSpeedNormal);
		EnhancedInputComponent->BindAction(SetRewindSpeedFasterAction, ETriggerEvent::Started, this, &ARewindCharacter::SetRewindSpeedFaster);
		EnhancedInputComponent->BindAction(SetRewindSpeedFastestAction, ETriggerEvent::Started, this, &ARewindCharacter::SetRewindSpeedFastest);
		EnhancedInputComponent->BindAction(ToggleRewindPartipationAction, ETriggerEvent::Started, this, &ARewindCharacter::ToggleRewindParticipation);
		EnhancedInputComponent->BindAction(ToggleTimelineVisualizationAction, ETriggerEvent::Started, this, &ARewindCharacter::ToggleTimelineVisualization);
	}
	else
	{
		UE_LOG(
			LogTemplateCharacter,
			Error,
			TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend "
		         "to use the legacy system, then you will need to update this C++ file."),
			*GetNameSafe(this));
	}
}

void ARewindCharacter::Jump()
{
	// Ignore input while manipulating time
	if (RewindComponent->IsTimeBeingManipulated()) { return; }
	Super::Jump();
}

void ARewindCharacter::StopJumping()
{
	// Ignore input while manipulating time
	if (RewindComponent->IsTimeBeingManipulated()) { return; }
	Super::StopJumping();
}

void ARewindCharacter::Move(const FInputActionValue& Value)
{
	// Ignore input while manipulating time
	if (RewindComponent->IsTimeBeingManipulated()) { return; }
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARewindCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (RewindComponent->IsTimeBeingManipulated())
	{
		FRotator NewRotation = CameraBoom->GetRelativeRotation();
		NewRotation.Yaw += LookAxisVector.X;
		NewRotation.Pitch -= LookAxisVector.Y;
		NewRotation.Pitch = FMath::ClampAngle(NewRotation.Pitch, -80.0f, 80.0f);
		CameraBoom->SetRelativeRotation(NewRotation);
	}
	else if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARewindCharacter::ToggleTimeScrub(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->ToggleTimeScrub(); }
}

void ARewindCharacter::Rewind(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->StartGlobalRewind(); }
}

void ARewindCharacter::StopRewinding(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->StopGlobalRewind(); }
}

void ARewindCharacter::FastForward(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->StartGlobalFastForward(); }
}

void ARewindCharacter::StopFastForwarding(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->StopGlobalFastForward(); }
}

void ARewindCharacter::SetRewindSpeedSlowest(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->SetRewindSpeedSlowest(); }
}

void ARewindCharacter::SetRewindSpeedSlower(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->SetRewindSpeedSlower(); }
}

void ARewindCharacter::SetRewindSpeedNormal(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->SetRewindSpeedNormal(); }
}

void ARewindCharacter::SetRewindSpeedFaster(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->SetRewindSpeedFaster(); }
}

void ARewindCharacter::SetRewindSpeedFastest(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->SetRewindSpeedFastest(); }
}

void ARewindCharacter::ToggleRewindParticipation(const FInputActionValue& Value)
{
	RewindComponent->SetIsRewindingEnabled(!RewindComponent->IsRewindingEnabled());
}

void ARewindCharacter::ToggleTimelineVisualization(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->ToggleGlobalTimelineVisualization(); }
}

void ARewindCharacter::UpdateCamera()
{
	if (RewindComponent->IsTimeBeingManipulated())
	{
		// Switch to bird's eye camera view
		CameraBoom->bUsePawnControlRotation = false;
		CameraBoom->TargetArmLength = RewindTargetCameraArmLength;
	}
	else
	{
		// Restore original camera view
		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->TargetArmLength = OriginalTargetCameraArmLength;
	}
}
