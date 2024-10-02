// © Artem Podorozhko. All Rights Reserved. This project, including all associated assets, code, and content, is the property of Artem Podorozhko. Unauthorized use, distribution, or modification is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AdvancedMovementComponent.generated.h"

UENUM(Blueprintable, BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName="Slide"),
	CMOVE_MAX UMETA(Hidden)
};

class UAdvancedMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXMC_Action, UAdvancedMovementComponent*, MovementComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FXMC_DashAction, UAdvancedMovementComponent*, MovementComponent,
	uint8, DashDirection);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FXMC_ActionMovementMode, UAdvancedMovementComponent*, MovementComponent,
	EMovementMode, PrevMode, uint8, PrevCustomMode);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ADVANCEDMOVEMENT_API UAdvancedMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	friend class AAdvancedMovementCharacter;

	enum EDashDirection
	{
		DASH_None = 0,
		DASH_Left = 1,
		DASH_Right = 2,
		DASH_Forward = 3,
		DASH_Backward = 4,
		DASH_Max,
	};

public:
	class FSavedMove_Advanced : public FSavedMove_Character
	{
	public:
		enum CompressedFlags
		{
			FLAG_Sprint = 0x10,
			FLAG_Slide = 0x20,
			FLAG_Dash = 0x40,
			FLAG_Custom = 0x80
		};

		FSavedMove_Advanced();

		// Flag (Replicated)
		uint8 Saved_bWantsToSprint:1;
		uint8 Saved_bWantsToSlide:1;
		uint8 Saved_bWantsToDash:1;

		// Not a flag
		uint8 Saved_bPrevWantsToCrouch:1;
		//uint8 Saved_bWantsToProne:1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
			FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_Advanced : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Advanced(const UCharacterMovementComponent& ClientMovement);
		virtual FSavedMovePtr AllocateNewMove() override;

	};

public:
	UAdvancedMovementComponent();

protected:
	bool Safe_bWantsToSprint{ false };
	bool Safe_bWantsToSlide{ false };
	bool Safe_bPrevWantsToCrouch{ false };

	bool Safe_bWantsToDash{ false };
	
	FTimerHandle TimerHandle_DashCooldown;
	float DashStartTime;

	UPROPERTY(Transient)
	class AAdvancedMovementCharacter* AdvancedCharacter;

	UPROPERTY(ReplicatedUsing=OnRep_DashStart)
	bool Proxy_bDashStart;

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Walk")
	float Sprint_MaxSpeed{ 550.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Walk")
	float Walk_MaxSpeed{ 300.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_MinSpeed{ 200.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_MaxSpeed{ 600.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_MaxBrakingDeceleration{ 2048.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_EnterImpulse{ 500.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_GravityForce{ 5000.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	float Slide_Friction{ 1.3f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Slide")
	bool Slide_ResetVelocity{ false };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Prone")
	TArray<TEnumAsByte<EObjectTypeQuery>> Prone_ScanObjectTypes;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Forward)",
		Category="Movement|Dash")
	float Dash_Impulse_F{ 100.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Backward)",
		Category="Movement|Dash")
	float Dash_Impulse_B{ 100.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Right)",
		Category="Movement|Dash")
	float Dash_Impulse_R{ 100.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, DisplayName="Dash Impulse (Left)",
		Category="Movement|Dash")
	float Dash_Impulse_L{ 100.0f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Dash")
	float Dash_CooldownDuration{ 1.f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Movement|Dash")
	float Dash_AuthCooldownDuration{ .9f };

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual UCapsuleComponent* GetOwnerCapsuleComponent() const;
	
	virtual void InitializeComponent() override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	// After all movement updated
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;


	UFUNCTION(BlueprintCallable)
	bool IsSprintingAllowed() const;

	virtual void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	virtual void ExitSlide();
	virtual bool CanSlide() const;
	virtual void PhysSlide(float DeltaTime, int32 Iterations);
	virtual bool GetSlideSurface(FHitResult& Hit) const;

	virtual bool CanDash() const;
	virtual void PerformDash();
	virtual void OnDashCooldownFinished();

	virtual EDashDirection CalculateDashDirection(float InAngle) const;
	virtual float CalculateDirection() const;

	UFUNCTION()
	virtual void OnRep_DashStart();

public:
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool IsSliding() const;

	UFUNCTION(BlueprintCallable)
	virtual void SprintPressed();

	UFUNCTION(BlueprintCallable)
	virtual void SprintReleased();

	UFUNCTION(BlueprintCallable)
	virtual void CrouchPressed();

	UFUNCTION(BlueprintCallable)
	virtual void CrouchReleased();

	UFUNCTION(BlueprintCallable)
	virtual void SlidePressed();

	UFUNCTION(BlueprintCallable)
	virtual void SlideReleased();

	UFUNCTION(BlueprintCallable)
	virtual void DashPressed();

	UFUNCTION(BlueprintCallable)
	virtual void DashReleased();

	virtual bool IsSprinting() const { return Safe_bWantsToSprint; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual float GetSprintMaxSpeed() const { return Sprint_MaxSpeed; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	virtual bool IsMovementMode(EMovementMode InMovementMode) const;


	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, DisplayName="On started sliding")
	FXMC_ActionMovementMode OnEnteredSlide;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, DisplayName="On finished sliding")
	FXMC_ActionMovementMode OnLeftSlide;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, DisplayName="On dashed")
	FXMC_DashAction OnDashStarted;
};
