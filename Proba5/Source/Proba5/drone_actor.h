// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "my_serial.h"
//#include "Components/SkeletalMeshComponent.h"
#include "drone_actor.generated.h"


UCLASS()
class PROBA5_API Adrone_actor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	Adrone_actor();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial")
	TArray<uint8> SerialBuf;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelX = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelY = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelZ = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GyroX = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GyroY = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GyroZ = 0.0f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Declare serial1 as a class member
	my_serial serial1;
};
