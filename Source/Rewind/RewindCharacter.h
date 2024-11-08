// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"

#include "RewindCharacter.generated.h"

class ARewindGameMode;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class URewindComponent;
class URewindVisualizationComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class ARewindCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARewindCharacter();
	virtual void Jump() override;
	virtual void StopJumping() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
	float RewindTargetCameraArmLength = 2000.0f;

protected:
	virtual void BeginPlay();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleTimeScrub(const FInputActionValue& Value);
	void Rewind(const FInputActionValue& Value);
	void StopRewinding(const FInputActionValue& Value);
	void FastForward(const FInputActionValue& Value);
	void StopFastForwarding(const FInputActionValue& Value);
	void SetRewindSpeedSlowest(const FInputActionValue& Value);
	void SetRewindSpeedSlower(const FInputActionValue& Value);
	void SetRewindSpeedNormal(const FInputActionValue& Value);
	void SetRewindSpeedFaster(const FInputActionValue& Value);
	void SetRewindSpeedFastest(const FInputActionValue& Value);
	void ToggleRewindParticipation(const FInputActionValue& Value);
	void ToggleTimelineVisualization(const FInputActionValue& Value);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind", meta = (AllowPrivateAccess = "true"))
	URewindComponent* RewindComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind", meta = (AllowPrivateAccess = "true"))
	URewindVisualizationComponent* RewindVisualizationComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleTimeScrubAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RewindAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FastForwardAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedSlowestAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedSlowerAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedNormalAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedFasterAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedFastestAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleRewindPartipationAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleTimelineVisualizationAction;

	// Original camera arm length; restored after rewind view
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind | Debug")
	float OriginalTargetCameraArmLength = 500.0f;

	// Game mode for driving global time manipulation operations
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	ARewindGameMode* GameMode;
	
	UFUNCTION()
	void UpdateCamera();


public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
};
