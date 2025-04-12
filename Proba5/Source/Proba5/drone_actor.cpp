// Fill out your copyright notice in the Description page of Project Settings.


#include "drone_actor.h"


// Sets default values
Adrone_actor::Adrone_actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Convert accelerometer data to orientation quaternion
FQuat GetOrientationFromAccel(float accelx, float accely, float accelz)
{
    // Step 1: Create accelerometer vector
    FVector Accel(accelx, accely, accelz);
    FVector Ref = FVector(0.0f, 0.0f, 1.0f); // Reference "up" vector

    // Step 2: Normalize accelerometer vector
    FVector AccelNorm = Accel.GetSafeNormal();

    // Step 3: Compute axis and angle
    FVector Axis = FVector::CrossProduct(AccelNorm, Ref);
    float Dot = FVector::DotProduct(AccelNorm, Ref);
    Dot = FMath::Clamp(Dot, -1.0f, 1.0f); // Clamp for acos safety
    float Angle = FMath::Acos(Dot);

    // Step 4: Convert axis-angle to quaternion
    if (Axis.IsNearlyZero()) {
        // If axis is zero (vectors are aligned), return identity
        return FQuat::Identity;
    }

    Axis.Normalize();
    return FQuat(Axis, Angle);
}

FQuat GetRotationFromGyro(float gyrox, float gyroy, float gyroz, float DeltaTime)
{
    FVector AngularVelocity(gyrox, gyroy, gyroz); // radians/sec
    float Angle = AngularVelocity.Size() * DeltaTime;
    
    if (Angle < KINDA_SMALL_NUMBER)
    {
        return FQuat::Identity;
    }

    FVector Axis = AngularVelocity.GetSafeNormal(); // unit axis
    return FQuat(Axis, Angle);
}

// Called when the game starts or when spawned
void Adrone_actor::BeginPlay()
{
	Super::BeginPlay();
	SerialBuf.SetNum(256);  // Allocate 256 bytes
	UE_LOG(LogTemp, Warning, TEXT("GAME STARTED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	serial1.my_serial_open();
	
}

// Called every frame
void Adrone_actor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (serial1.isOpen())
    {
        if (SerialBuf.Num() < 256)
        {
            SerialBuf.SetNum(256, false);
        }
        serial1.my_serial_read(reinterpret_cast<char*>(SerialBuf.GetData()), 256);
    }

    // Convert the byte array to FString
    char* dataPtr = reinterpret_cast<char*>(SerialBuf.GetData());
    dataPtr[SerialBuf.Num() - 1] = '\0';
    FString ReceivedData(ANSI_TO_TCHAR(dataPtr));

    FRotator accel_rotation = FRotator::ZeroRotator;

    FQuat Orientation;
    FRotator Rot;
    FQuat DeltaRotation;
    FRotator DeltaRotator;

    // Extract Accelerometer Data
    FString AccelData;
    if (ReceivedData.Split(TEXT("AccelX:"), nullptr, &AccelData))
    {
        FString AccelYPart, AccelZPart;
        if (AccelData.Split(TEXT("Y:"), &AccelData, &AccelYPart) &&
            AccelYPart.Split(TEXT("Z:"), &AccelYPart, &AccelZPart))
        {
            AccelX = FCString::Atof(*AccelData);
            AccelY = FCString::Atof(*AccelYPart);
            AccelZ = FCString::Atof(*AccelZPart);

            // Get accelerometer orientation quaternion
            Orientation = GetOrientationFromAccel(AccelX, AccelY, AccelZ);
            Rot = Orientation.Rotator(); // If you want to use it as a Rotator

            UE_LOG(LogTemp, Log, TEXT("Orientation Quaternion: W=%.4f, X=%.4f, Y=%.4f, Z=%.4f"), Orientation.W, Orientation.X, Orientation.Y, Orientation.Z);
            UE_LOG(LogTemp, Log, TEXT("Orientation Rotator: Pitch=%.2f, Yaw=%.2f, Roll=%.2f"), Rot.Pitch, Rot.Yaw, Rot.Roll);
        }
    }

    // Extract Gyroscope Data
    FString GyroData;
    if (ReceivedData.Split(TEXT("GyroX:"), nullptr, &GyroData))
    {
        FString GyroYPart, GyroZPart;
        if (GyroData.Split(TEXT("Y:"), &GyroData, &GyroYPart) &&
            GyroYPart.Split(TEXT("Z:"), &GyroYPart, &GyroZPart))
        {
            GyroX = FCString::Atof(*GyroData);
            GyroY = FCString::Atof(*GyroYPart);
            GyroZ = FCString::Atof(*GyroZPart);

            // Get the rotational delta from gyro data
            DeltaRotation = GetRotationFromGyro(GyroX, GyroY, GyroZ, DeltaTime);
            //DeltaRotator = DeltaRotation.Rotator();

            //UE_LOG(LogTemp, Log, TEXT("Gyro Quaternion Delta: W=%.4f, X=%.4f, Y=%.4f, Z=%.4f"), DeltaRotation.W, DeltaRotation.X, DeltaRotation.Y, DeltaRotation.Z);
            //UE_LOG(LogTemp, Log, TEXT("Gyro Rotator Delta: Pitch=%.2f, Yaw=%.2f, Roll=%.2f"), DeltaRotator.Pitch, DeltaRotator.Yaw, DeltaRotator.Roll);
        }
    }

    // Apply the complementary filter
    // Predicted orientation from gyro
    FQuat GyroPrediction = GyroPrediction + DeltaRotator;
    GyroPrediction.Normalize();

    // Orientation from accelerometer
    FQuat AccelQuat = Orientation;  // Already set from accel data

    // Use complementary filter formula (Manual blending)
    float Alpha = 0.0f; // You can adjust this value based on your sensor needs
    FQuat BlendedQuat;

    BlendedQuat.X = Alpha * GyroPrediction.X + (1.0f - Alpha) * AccelQuat.X;
    BlendedQuat.Y = Alpha * GyroPrediction.Y + (1.0f - Alpha) * AccelQuat.Y;
    BlendedQuat.Z = Alpha * GyroPrediction.Z + (1.0f - Alpha) * AccelQuat.Z;
    BlendedQuat.W = Alpha * GyroPrediction.W + (1.0f - Alpha) * AccelQuat.W;

    BlendedQuat.Normalize(); // Normalize the blended quaternion to keep it valid

    // Set Actor Rotation
    SetActorRotation(BlendedQuat.Rotator());

    // Optional: Debugging the resulting rotation
    FRotator FinalRot = BlendedQuat.Rotator();
    //UE_LOG(LogTemp, Log, TEXT("Final Filtered Rotator: Pitch=%.2f, Yaw=%.2f, Roll=%.2f"), FinalRot.Pitch, FinalRot.Yaw, FinalRot.Roll);
}

void Adrone_actor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	serial1.my_serial_close();
	UE_LOG(LogTemp, Warning, TEXT("Game closed!"));

	Super::EndPlay(EndPlayReason);
}
