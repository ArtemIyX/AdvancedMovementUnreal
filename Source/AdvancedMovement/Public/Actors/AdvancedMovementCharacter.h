// © Artem Podorozhko. All Rights Reserved. This project, including all associated assets, code, and content, is the property of Artem Podorozhko. Unauthorized use, distribution, or modification is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AdvancedMovementCharacter.generated.h"

/**
 * @brief A character class for handling advanced movement features.
 * 
 * The AAdvancedMovementCharacter class extends the base ACharacter class 
 * to include advanced movement capabilities such as sliding, dashing, and 
 * view manipulation. This class integrates with UAdvancedMovementComponent 
 * to provide enhanced player movement controls.
 */
UCLASS(Blueprintable, BlueprintType)
class ADVANCEDMOVEMENT_API AAdvancedMovementCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	/**
	 * @brief Default constructor that sets initial values for this character's properties.
	 * 
	 * @param ObjectInitializer Custom object initializer.
	 */
	AAdvancedMovementCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** 
	 * @brief The advanced movement component that handles specialized movement logic.
	 */
	UPROPERTY()
	class UAdvancedMovementComponent* AdvancedMovementComponent;

	/**
	 * @brief Maximum yaw angle allowed during slide movement.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="AAdvancedMovementCharacter|Movement|Slide")
	float SlideMaxYawView{ 20.0f };

protected:


public:

protected:

	/**
	 * @brief Event handler for when the slide movement is entered.
	 * 
	 * @param MovementComponent The movement component controlling the character.
	 * @param PrevMode The previous movement mode.
	 * @param PrevCustomMode The previous custom movement mode.
	 */
	UFUNCTION()
	virtual void OnSlideEnteredHandler(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);

	/**
	 * @brief Blueprint implementable event triggered when the slide starts.
	 * 
	 * @param MovementComponent The movement component controlling the character.
	 * @param PrevMode The previous movement mode.
	 * @param PrevCustomMode The previous custom movement mode.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName="Slide Started", Category="AAdvancedMovementCharacter|Movement")
	void BP_EnteredSlide(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);

	/**
	 * @brief Event handler for when the slide movement is left.
	 * 
	 * @param MovementComponent The movement component controlling the character.
	 * @param PrevMode The previous movement mode.
	 * @param PrevCustomMode The previous custom movement mode.
	 */
	UFUNCTION()
	virtual void OnSlideLeftHandler(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);

	/**
	 * @brief Blueprint implementable event triggered when the slide finishes.
	 * 
	 * @param MovementComponent The movement component controlling the character.
	 * @param PrevMode The previous movement mode.
	 * @param PrevCustomMode The previous custom movement mode.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName="Slide Finished", Category="AAdvancedMovementCharacter|Movement")
	void BP_LeftSlide(UAdvancedMovementComponent* MovementComponent, EMovementMode PrevMode, uint8 PrevCustomMode);

	/**
	 * @brief Event handler for when a dash movement starts.
	 * 
	 * @param MovementComponent The movement component controlling the character.
	 * @param DashDirection The direction of the dash.
	 */
	UFUNCTION()
	virtual void OnDashStartedHandler(UAdvancedMovementComponent* MovementComponent, uint8 DashDirection);

	/**
	 * @brief Blueprint implementable event triggered when a dash starts.
	 * 
	 * @param MovementComponent The movement component controlling the character.
	 * @param DashDirection The direction of the dash.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName="Dash Started", Category="AAdvancedMovementCharacter|Movement")
	void BP_DashStarted(UAdvancedMovementComponent* MovementComponent, uint8 DashDirection);

protected:

	/**
	 * @brief Limits the character's yaw (horizontal rotation) to the specified range.
	 * 
	 * @param YawMin The minimum yaw value.
	 * @param YawMax The maximum yaw value.
	 */
	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual void LimitYaw(float YawMin = 0.0f, float YawMax = 359.9899f);

	/**
	 * @brief Limits the character's pitch (vertical rotation) to the specified range.
	 * 
	 * @param PitchMin The minimum pitch value.
	 * @param PitchMax The maximum pitch value.
	 */
	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual void LimitPitch(float PitchMin = -89.900002f, float PitchMax = 89.900002f);

	/**
	 * @brief Calculates yaw freezing based on the input value and deviation.
	 * 
	 * @param InValue The current yaw value.
	 * @param Deviation The allowed deviation in yaw.
	 * @return A 2D vector representing the yaw freeze range.
	 */
	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual FVector2D CalculateYawFreeze(float InValue, int Deviation) const;

	/**
	 * @brief Calculates pitch freezing based on the input value and deviation.
	 * 
	 * @param InValue The current pitch value.
	 * @param Deviation The allowed deviation in pitch.
	 * @return A 2D vector representing the pitch freeze range.
	 */
	UFUNCTION(BlueprintCallable, Category="AAdvancedMovementCharacter|View")
	virtual FVector2D CalculatePitchFreeze(float InValue, int Deviation) const;

public:

	/**
	 * @brief Gets the collision query parameters that ignore the character.
	 * 
	 * @return FCollisionQueryParams The collision query parameters that ignore the character.
	 */
	FCollisionQueryParams GetIgnoreCharacterParams() const;

	UFUNCTION(BlueprintNativeEvent, Category="AAdvancedMovementCharacter|Movement")
	bool CanDash();
};