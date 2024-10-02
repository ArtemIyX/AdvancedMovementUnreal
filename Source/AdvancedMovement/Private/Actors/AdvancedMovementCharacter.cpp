﻿// © Artem Podorozhko. All Rights Reserved. This project, including all associated assets, code, and content, is the property of Artem Podorozhko. Unauthorized use, distribution, or modification is strictly prohibited.


#include "Actors/AdvancedMovementCharacter.h"

#include "Components/AdvancedMovementComponent.h"


// Sets default values
AAdvancedMovementCharacter::AAdvancedMovementCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAdvancedMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AdvancedMovementComponent = Cast<UAdvancedMovementComponent>(GetCharacterMovement());
	AdvancedMovementComponent->SetIsReplicated(true);
	AdvancedMovementComponent->OnEnteredSlide.AddDynamic(this, &AAdvancedMovementCharacter::OnSlideEnteredHandler);
	AdvancedMovementComponent->OnLeftSlide.AddDynamic(this, &AAdvancedMovementCharacter::OnSlideLeftHandler);
	AdvancedMovementComponent->OnDashStarted.AddDynamic(this, &AAdvancedMovementCharacter::OnDashStartedHandler);
}

// Called when the game starts or when spawned
void AAdvancedMovementCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAdvancedMovementCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAdvancedMovementCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAdvancedMovementCharacter::OnSlideEnteredHandler(UAdvancedMovementComponent* MovementComponent,
                                                       EMovementMode PrevMode, uint8 PrevCustomMode)
{
	const FVector2D freeze = CalculateYawFreeze(
		GetControlRotation().Yaw, SlideMaxYawView);
	LimitYaw(freeze.X, freeze.Y);

	BP_EnteredSlide(MovementComponent, PrevMode, PrevCustomMode);
	// Play montage etc..
}

void AAdvancedMovementCharacter::OnSlideLeftHandler(UAdvancedMovementComponent* MovementComponent,
                                                    EMovementMode PrevMode, uint8 PrevCustomMode)
{
	LimitYaw();
	BP_LeftSlide(MovementComponent, PrevMode, PrevCustomMode);
}

void AAdvancedMovementCharacter::OnDashStartedHandler(UAdvancedMovementComponent* MovementComponent,
                                                      uint8 DashDirection)
{
	BP_DashStarted(MovementComponent, DashDirection);
	// Play montage etc..
}

void AAdvancedMovementCharacter::LimitYaw(float YawMin, float YawMax)
{
	if (Controller)
	{
		if (APlayerController* pc = Cast<APlayerController>(Controller))
		{
			if (pc->PlayerCameraManager)
			{
				pc->PlayerCameraManager->ViewYawMin = YawMin;
				pc->PlayerCameraManager->ViewYawMax = YawMax;
			}
		}
	}
}

void AAdvancedMovementCharacter::LimitPitch(float PitchMin, float PitchMax)
{
	if (Controller)
	{
		if (APlayerController* pc = Cast<APlayerController>(Controller))
		{
			if (pc->PlayerCameraManager)
			{
				pc->PlayerCameraManager->ViewPitchMin = PitchMin;
				pc->PlayerCameraManager->ViewPitchMax = PitchMax;
			}
		}
	}
}

FVector2D AAdvancedMovementCharacter::CalculateYawFreeze(float InValue, int Deviation) const
{
	const int minAngle = (static_cast<int>(InValue) - Deviation + 360) % 360;
	const int maxAngle = (static_cast<int>(InValue) + Deviation) % 360;

	return FVector2D(minAngle, maxAngle);
}

FVector2D AAdvancedMovementCharacter::CalculatePitchFreeze(float InValue, int Deviation) const
{
	return FVector2D(
		FMath::Clamp(InValue - Deviation, -89.900002f, 89.900002f),
		FMath::Clamp(InValue + Deviation, -89.900002f, 89.900002f)
	);
}

FCollisionQueryParams AAdvancedMovementCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams params;

	TArray<AActor*> children;
	GetAllChildActors(children);
	params.AddIgnoredActors(children);
	params.AddIgnoredActor(this);
	return params;
}
