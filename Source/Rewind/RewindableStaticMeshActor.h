// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/StaticMeshActor.h"

#include "RewindableStaticMeshActor.generated.h"

class URewindComponent;
class URewindVisualizationComponent;

/**
 * Static mesh actor that supports time rewinding
 */
UCLASS()
class REWIND_API ARewindableStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	
	ARewindableStaticMeshActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	URewindComponent* RewindComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	URewindVisualizationComponent* RewindVisualizationComponent;

};
