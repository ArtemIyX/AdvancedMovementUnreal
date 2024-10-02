// © Artem Podorozhko. All Rights Reserved. This project, including all associated assets, code, and content, is the property of Artem Podorozhko. Unauthorized use, distribution, or modification is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AdvancedMovementCharacter.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ADVANCEDMOVEMENT_API AAdvancedMovementCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AAdvancedMovementCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AAdvancedMovementCharacter|Components")
	class UAdvancedMovementComponent* AdvancedMovementComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly,
	Category="AAdvancedMovementCharacter|Movement|Slide")
	float SlideMaxYawView{ 20.0f };
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	UFUNCTION()
	virtual void OnSlideEnteredHandler(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="Slide Started", Category="AAdvancedMovementCharacter|Movement")
	void BP_EnteredSlide(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);
	
	UFUNCTION()
	virtual void OnSlideLeftHandler(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="Slide Finished", Category="AAdvancedMovementCharacter|Movement")
	void BP_LeftSlide(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);

	UFUNCTION()
	virtual void OnDashStartedHandler(UAdvancedMovementComponent* MovementComponent, uint8 DashDirection);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="Dash Started", Category="AAdvancedMovementCharacter|Movement")
	void BP_DashStarted(UAdvancedMovementComponent* MovementComponent, uint8 DashDirection);

protected:
	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual void LimitYaw(float YawMin = 0.0f, float YawMax = 359.9899f);

	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual void LimitPitch(float PitchMin = -89.900002f, float PitchMax = 89.900002f);

	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual FVector2D CalculateYawFreeze(float InValue, int Deviation) const;

	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual FVector2D CalculatePitchFreeze(float InValue, int Deviation) const;

public:

	FCollisionQueryParams GetIgnoreCharacterParams() const;
};
