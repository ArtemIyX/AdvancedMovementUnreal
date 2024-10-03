// © Artem Podorozhko. All Rights Reserved. This project, including all associated assets, code, and content, is the property of Artem Podorozhko. Unauthorized use, distribution, or modification is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AdvancedMovementComponent.generated.h"

/**
 * @brief Custom movement modes for the AdvancedMovementComponent.
 */
UENUM(Blueprintable, BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden), /**< No custom movement mode. */
	CMOVE_Slide UMETA(DisplayName="Slide"), /**< Sliding movement mode. */
	CMOVE_MAX UMETA(Hidden) /**< Maximum limit for custom movement modes. */
};

class UAdvancedMovementComponent;

/**
 * @brief Delegate for actions involving UAdvancedMovementComponent.
 * 
 * @param MovementComponent The movement component that triggered the action.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXMC_Action, UAdvancedMovementComponent*, MovementComponent);

/**
 * @brief Delegate for dash actions.
 * 
 * @param MovementComponent The movement component that triggered the action.
 * @param DashDirection The direction of the dash.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FXMC_DashAction, UAdvancedMovementComponent*, MovementComponent, uint8,
                                             DashDirection);

/**
 * @brief Delegate for movement mode changes.
 * 
 * @param MovementComponent The movement component that triggered the action.
 * @param PrevMode The previous movement mode.
 * @param PrevCustomMode The previous custom movement mode.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FXMC_ActionMovementMode, UAdvancedMovementComponent*, MovementComponent,
                                               EMovementMode, PrevMode, uint8, PrevCustomMode);

/**
 * @class UAdvancedMovementComponent
 * @brief Custom character movement component that adds advanced movement features such as sprinting, sliding, and dashing.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ADVANCEDMOVEMENT_API UAdvancedMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	/**
	 * @brief Enumeration of available dash directions.
	 */
	enum EDashDirection
	{
		DASH_None = 0, /**< No dash. */
		DASH_Left = 1, /**< Dash to the left. */
		DASH_Right = 2, /**< Dash to the right. */
		DASH_Forward = 3, /**< Dash forward. */
		DASH_Backward = 4, /**< Dash backward. */
		DASH_Max /**< Maximum limit for dash directions. */
	};

	/**
	 * @brief Class to save advanced movement states for networking.
	 */
	class FSavedMove_Advanced : public FSavedMove_Character
	{
	public:
		/** Flags for the advanced movement state. */
		enum CompressedFlags
		{
			FLAG_Sprint = 0x10, /**< Sprinting flag. */
			FLAG_Slide = 0x20, /**< Sliding flag. */
			FLAG_Dash = 0x40, /**< Dashing flag. */
			FLAG_Custom = 0x80 /**< Custom movement flag. */
		};

		FSavedMove_Advanced();

		/**
         * @brief Indicates if the character wants to sprint.
         */
		uint8 Saved_bWantsToSprint : 1;

		/**
         * @brief Indicates if the character wants to slide.
         */
		uint8 Saved_bWantsToSlide : 1;

		/**
         * @brief Indicates if the character wants to dash.
         */
		uint8 Saved_bWantsToDash : 1;

		/**
         * @brief Indicates if the character previously wanted to crouch.
         */
		uint8 Saved_bPrevWantsToCrouch : 1;
		
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
		                        FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	/**
	 * @brief Client-side prediction data for advanced movement.
	 */
	class FNetworkPredictionData_Client_Advanced : public FNetworkPredictionData_Client_Character
	{
	public:
		/**
         * @brief Constructor for advanced movement prediction data.
         * @param ClientMovement The character movement component.
         */
		FNetworkPredictionData_Client_Advanced(const UCharacterMovementComponent& ClientMovement);
		
		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	UAdvancedMovementComponent();

protected:
	/** 
	 * @brief Safe flag indicating whether the character wants to sprint.
	 */
	bool Safe_bWantsToSprint{false};

	/** 
	 * @brief Safe flag indicating whether the character wants to slide.
	 */
	bool Safe_bWantsToSlide{false};

	/** 
	 * @brief Safe flag indicating whether the character previously wanted to crouch.
	 */
	bool Safe_bPrevWantsToCrouch{false};

	/** 
	 * @brief Safe flag indicating whether the character wants to dash.
	 */
	bool Safe_bWantsToDash{false};

	/** 
	 * @brief Timer handle for the dash cooldown.
	 */
	FTimerHandle TimerHandle_DashCooldown;

	/** 
	 * @brief The start time of the dash.
	 */
	float DashStartTime;

	/** 
	 * @brief The owning character of this movement component.
	 */
	UPROPERTY(Transient)
	class AAdvancedMovementCharacter* AdvancedCharacter;

	/** 
	 * @brief Replicated flag indicating if the dash started.
	 */
	UPROPERTY(ReplicatedUsing=OnRep_DashStart)
	bool Proxy_bDashStart;

protected:
	/** 
     * @brief The maximum sprint speed.
     */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Walk")
	float Sprint_MaxSpeed{550.0f};

	// /** 
	//  * @brief The maximum walk speed.
	//  */
	// UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Walk")
	// float Walk_MaxSpeed{300.0f};

	/** 
	 * @brief The minimum speed during a slide.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_MinSpeed{200.0f};

	/** 
	 * @brief The maximum speed during a slide.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_MaxSpeed{600.0f};

	/** 
	 * @brief The maximum braking deceleration during a slide.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_MaxBrakingDeceleration{2048.0f};

	/** 
	 * @brief The initial impulse applied when entering a slide.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_EnterImpulse{500.0f};

	/** 
	 * @brief The gravitational force applied during a slide.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_GravityForce{5000.0f};

	/** 
	 * @brief The friction applied during a slide.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_Friction{1.3f};

	/** 
	 * @brief Flag indicating if velocity should be reset when starting a slide.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	bool Slide_ResetVelocity{false};

	/** 
	 * @brief Dash impulse when moving forward.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Forward)", Category="Movement|Dash")
	float Dash_Impulse_F{100.0f};

	/** 
	 * @brief Dash impulse when moving backward.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Backward)", Category="Movement|Dash")
	float Dash_Impulse_B{100.0f};

	/** 
	 * @brief Dash impulse when moving to the right.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Right)", Category="Movement|Dash")
	float Dash_Impulse_R{100.0f};

	/** 
	 * @brief Dash impulse when moving to the left.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Left)", Category="Movement|Dash")
	float Dash_Impulse_L{100.0f};

	/** 
	 * @brief Cooldown duration for dashing.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Dash")
	float Dash_CooldownDuration{1.f};

	/** 
	 * @brief Cooldown duration for dashing, as authorized by the server.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Dash")
	float Dash_AuthCooldownDuration{.9f};

protected:

	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	/**
	 * @brief Checks if sprinting is allowed.
	 * 
	 * @return True if sprinting is allowed, otherwise false.
	 */
	UFUNCTION(BlueprintCallable)
	bool IsSprintingAllowed() const;

	/**
	 * @brief Enters the sliding state.
	 * 
	 * @param PrevMode The previous movement mode.
	 * @param PrevCustomMode The previous custom movement mode.
	 */
	virtual void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);

	/**
	 * @brief Exits the sliding state.
	 */
	virtual void ExitSlide();

	/**
	 * @brief Checks if the character can slide.
	 * 
	 * @return True if sliding is allowed, otherwise false.
	 */
	virtual bool CanSlide() const;

	/**
	 * @brief Applies physics for sliding.
	 * 
	 * @param DeltaTime The time elapsed since the last update.
	 * @param Iterations The number of movement iterations.
	 */
	virtual void PhysSlide(float DeltaTime, int32 Iterations);

	/**
	 * @brief Gets the surface the character is sliding on.
	 * 
	 * @param Hit The result of the surface check.
	 * @return True if a valid surface is found, otherwise false.
	 */
	virtual bool GetSlideSurface(FHitResult& Hit) const;

	/**
	 * @brief Checks if the character can dash.
	 * 
	 * @return True if dashing is allowed, otherwise false.
	 */
	virtual bool CanDash() const;

	/**
	 * @brief Performs a dash.
	 */
	virtual void PerformDash();

	/**
	 * @brief Called when the dash cooldown finishes.
	 */
	virtual void OnDashCooldownFinished();

	/**
	 * @brief Calculates the dash direction based on the input angle.
	 * 
	 * @param InAngle The input angle.
	 * @return The dash direction.
	 */
	virtual EDashDirection CalculateDashDirection(float InAngle) const;

	/**
	 * @brief Calculates the direction of movement based on the character's velocity and input.
	 * 
	 * @return The calculated direction in degrees.
	 */
	virtual float CalculateDirection() const;

	/**
	 * @brief Called when the dash start is replicated.
	 */
	UFUNCTION()
	virtual void OnRep_DashStart();

public:
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/**
	* @brief Checks if the character is sliding.
	* 
	* @return True if the character is sliding, otherwise false.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool IsSliding() const;

	 /**
    * @brief Called when the sprint button is pressed.
    */
    UFUNCTION(BlueprintCallable)
    virtual void SprintPressed();

    /**
    * @brief Called when the sprint button is released.
    */
    UFUNCTION(BlueprintCallable)
    virtual void SprintReleased();

    /**
    * @brief Called when the crouch button is pressed.
    */
    UFUNCTION(BlueprintCallable)
    virtual void CrouchPressed();

    /**
    * @brief Called when the crouch button is released.
    */
    UFUNCTION(BlueprintCallable)
    virtual void CrouchReleased();

    /**
    * @brief Called when the slide button is pressed.
    */
    UFUNCTION(BlueprintCallable)
    virtual void SlidePressed();

    /**
    * @brief Called when the slide button is released.
    */
    UFUNCTION(BlueprintCallable)
    virtual void SlideReleased();

    /**
    * @brief Called when the dash button is pressed.
    */
    UFUNCTION(BlueprintCallable)
    virtual void DashPressed();

    /**
    * @brief Called when the dash button is released.
    */
    UFUNCTION(BlueprintCallable)
    virtual void DashReleased();

    /**
    * @brief Checks if the character is sprinting.
    * 
    * @return True if the character is sprinting, otherwise false.
    */
    virtual bool IsSprinting() const { return Safe_bWantsToSprint; }

    /**
    * @brief Gets the maximum sprint speed.
    * 
    * @return The maximum sprint speed.
    */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    virtual float GetSprintMaxSpeed() const { return Sprint_MaxSpeed; }

    /**
    * @brief Checks if the character is in a custom movement mode.
    * 
    * @param InCustomMovementMode The custom movement mode to check.
    * @return True if the character is in the specified custom movement mode, otherwise false.
    */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    virtual bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

    /**
    * @brief Checks if the character is in a specific movement mode.
    * 
    * @param InMovementMode The movement mode to check.
    * @return True if the character is in the specified movement mode, otherwise false.
    */
    virtual bool IsMovementMode(EMovementMode InMovementMode) const;

    /** 
    * @brief Event triggered when the character starts sliding.
    */
    UPROPERTY(BlueprintReadOnly, BlueprintAssignable, DisplayName="On started sliding")
    FXMC_ActionMovementMode OnEnteredSlide;

    /** 
    * @brief Event triggered when the character finishes sliding.
    */
    UPROPERTY(BlueprintReadOnly, BlueprintAssignable, DisplayName="On finished sliding")
    FXMC_ActionMovementMode OnLeftSlide;

    /** 
    * @brief Event triggered when the character starts dashing.
    */
    UPROPERTY(BlueprintReadOnly, BlueprintAssignable, DisplayName="On dashed")
    FXMC_DashAction OnDashStarted;
};
