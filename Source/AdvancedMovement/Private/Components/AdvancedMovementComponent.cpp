// Developed by OIC


// ================================================= FSavedMove_Xeus =========================================================

#include "Components/AdvancedMovementComponent.h"

#include "AdvancedMovement.h"
#include "Actors/AdvancedMovementCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/LoggerLib.h"

UAdvancedMovementComponent::FSavedMove_Advanced::FSavedMove_Advanced()
{
	Saved_bWantsToSprint = 0;
	Saved_bWantsToSlide = 0;
	Saved_bPrevWantsToCrouch = 0;
	Saved_bWantsToDash = 0;
}

bool UAdvancedMovementComponent::FSavedMove_Advanced::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                     ACharacter* InCharacter,
                                                                     float MaxDelta) const
{
	FSavedMove_Advanced* NewXeusMove = static_cast<FSavedMove_Advanced*>(NewMove.Get());
	if (Saved_bWantsToSprint != NewXeusMove->Saved_bWantsToSprint)
	{
		return false;
	}
	if (Saved_bWantsToSlide != NewXeusMove->Saved_bWantsToSlide)
	{
		return false;
	}
	if (Saved_bWantsToDash != NewXeusMove->Saved_bWantsToDash)
	{
		return false;
	}
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UAdvancedMovementComponent::FSavedMove_Advanced::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;
	Saved_bWantsToSlide = 0;
	Saved_bWantsToDash = 0;

	Saved_bPrevWantsToCrouch = 0;
}

uint8 UAdvancedMovementComponent::FSavedMove_Advanced::GetCompressedFlags() const
{
	uint8 result = FSavedMove_Character::GetCompressedFlags();

	if (Saved_bWantsToSprint)
		result |= FSavedMove_Advanced::CompressedFlags::FLAG_Sprint;

	if (Saved_bWantsToSlide)
		result |= FSavedMove_Advanced::CompressedFlags::FLAG_Slide;

	if (Saved_bWantsToDash)
		result |= FSavedMove_Advanced::CompressedFlags::FLAG_Dash;

	return result;
}

void UAdvancedMovementComponent::FSavedMove_Advanced::SetMoveFor(ACharacter* C, float InDeltaTime,
                                                                 FVector const& NewAccel,
                                                                 FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UAdvancedMovementComponent* MovementComponent = Cast<UAdvancedMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToSprint = MovementComponent->Safe_bWantsToSprint;
	Saved_bWantsToSlide = MovementComponent->Safe_bWantsToSlide;
	Saved_bWantsToDash = MovementComponent->Safe_bWantsToDash;

	Saved_bPrevWantsToCrouch = MovementComponent->Safe_bPrevWantsToCrouch;
}

void UAdvancedMovementComponent::FSavedMove_Advanced::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
	UAdvancedMovementComponent* MovementComponent = Cast<UAdvancedMovementComponent>(C->GetCharacterMovement());

	MovementComponent->Safe_bWantsToSprint = Saved_bWantsToSprint;
	MovementComponent->Safe_bWantsToSlide = Saved_bWantsToSlide;
	MovementComponent->Safe_bWantsToDash = Saved_bWantsToDash;

	MovementComponent->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
}

UAdvancedMovementComponent::FNetworkPredictionData_Client_Advanced::FNetworkPredictionData_Client_Advanced(
	const UCharacterMovementComponent& ClientMovement)
	: FNetworkPredictionData_Client_Character(ClientMovement)
{
}

FSavedMovePtr UAdvancedMovementComponent::FNetworkPredictionData_Client_Advanced::AllocateNewMove()
{
	return MakeShared<FSavedMove_Advanced>();
}

// Sets default values for this component's properties
UAdvancedMovementComponent::UAdvancedMovementComponent(): DashStartTime(0), AdvancedCharacter(nullptr),
                                                          Proxy_bDashStart(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	NavAgentProps.bCanCrouch = true;
	bCanWalkOffLedges = true;
	bCanWalkOffLedgesWhenCrouching = true;
}


// Called when the game starts
void UAdvancedMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


void UAdvancedMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AdvancedCharacter = Cast<AAdvancedMovementCharacter>(GetOwner());
}


// Called every frame
void UAdvancedMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FNetworkPredictionData_Client* UAdvancedMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);
	if (ClientPredictionData == nullptr)
	{
		UAdvancedMovementComponent* MutableThis = const_cast<UAdvancedMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Advanced(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UAdvancedMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide)
	{
		ExitSlide();
		OnLeftSlide.Broadcast(this, PreviousMovementMode, PreviousCustomMode);
	}
	// if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Prone)
	// {
	// 	ExitProne();
	// }

	if (IsCustomMovementMode(CMOVE_Slide))
	{
		EnterSlide(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);
		OnEnteredSlide.Broadcast(this, PreviousMovementMode, PreviousCustomMode);
	}
	// if (IsCustomMovementMode(CMOVE_Prone))
	// {
	// 	EnterProne(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);
	// }
	CharacterOwner->OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UAdvancedMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UAdvancedMovementComponent, Proxy_bDashStart, COND_SkipOwner);
}

void UAdvancedMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bool bWantsSprint = (Flags & FSavedMove_Advanced::CompressedFlags::FLAG_Sprint) != 0;
	if (bWantsSprint)
	{
		if (IsSprintingAllowed())
		{
			Safe_bWantsToSprint = true;
		}
	}
	else
	{
		Safe_bWantsToSprint = false;
	}
	//UE_LOG(LogTemp, Warning, TEXT("UpdateFromCompressedFlags, Safe_bWantsToSprint: %d"), Safe_bWantsToSprint);

	Safe_bWantsToSlide = (Flags & FSavedMove_Advanced::CompressedFlags::FLAG_Slide) != 0;
	Safe_bWantsToDash = (Flags & FSavedMove_Advanced::CompressedFlags::FLAG_Dash) != 0;
}

void UAdvancedMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
                                                   const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

void UAdvancedMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Update before crouching update
	{
		if (MovementMode == MOVE_Walking && !bWantsToCrouch && Safe_bWantsToSlide)
		{
			if (CanSlide())
			{
				SetMovementMode(MOVE_Custom, CMOVE_Slide);
			}
		}

		if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
		{
			SetMovementMode(MOVE_Walking);
		}

		// if (Safe_bWantsToProne)
		// {
		// 	if (CanProne())
		// 	{
		// 		SetMovementMode(MOVE_Custom, CMOVE_Prone);
		// 		if (!CharacterOwner->HasAuthority())
		// 			Server_EnterProne();
		// 	}
		// 	Safe_bWantsToProne = false;
		// }
		// if (IsCustomMovementMode(CMOVE_Prone) && !bWantsToCrouch)
		// {
		// 	SetMovementMode(MOVE_Walking);
		// }


		const bool bAuthProxy = CharacterOwner->HasAuthority() && !CharacterOwner->IsLocallyControlled();
		const bool bEnoughTime = GetWorld()->GetTimeSeconds() - DashStartTime > Dash_AuthCooldownDuration;
		const bool bCanDash = CanDash();
		UE_LOG(LogTemp, Log, TEXT("Safe_bWantsToDash: %d, bCanDash: %d\n!bAuthProxy: %d || %d: bEnoughTime"),
		       Safe_bWantsToDash, bCanDash, bAuthProxy, bEnoughTime);
		if (Safe_bWantsToDash && !bCanDash)
		{
			Safe_bWantsToDash = false;
		}
		else if (Safe_bWantsToDash && bCanDash)
		{
			if (!bAuthProxy || bEnoughTime)
			{
				PerformDash();
				Safe_bWantsToDash = false;
				Proxy_bDashStart = !Proxy_bDashStart;
			}
			else
			{
				TRACEWARN(LogAdvancedMovement, "Client tried to cheat with dash");
			}
		}
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UAdvancedMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);
	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	// case CMOVE_Prone:
	// 	PhysProne(deltaTime, Iterations);
	// 	break;
	default:
		TRACEERROR(LogTemp, "Invalid movement mode");
	}
}


bool UAdvancedMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() ||
		IsCustomMovementMode(CMOVE_Slide);
}

bool UAdvancedMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float UAdvancedMovementComponent::GetMaxSpeed() const
{
	if (IsMovementMode(MOVE_Walking)
		&& Safe_bWantsToSprint
		&& !IsCrouching())
	{
		return Sprint_MaxSpeed;
	}


	if (MovementMode != MOVE_Custom)
		return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return Slide_MaxSpeed;
	default:
		TRACEERROR(LogAdvancedMovement, "Invalid Movement Mode");
		return 0.0f;
	}
	//return 0.0f;
}

float UAdvancedMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode != MOVE_Custom)
		return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return Slide_MaxBrakingDeceleration;
	// case CMOVE_Prone:
	// 	return Prone_MaxBrakingDeceleration;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
}

bool UAdvancedMovementComponent::IsSprintingAllowed() const
{
	return !IsCrouching() // Not crouching
		&& !IsFalling() // Not falling
		&& !IsCustomMovementMode(CMOVE_Slide)
		&& IsMovingOnGround() // Is moving on ground
		&& Velocity.SizeSquared() >= 100.0f // Velocity must be not 0.0f
		&& !Safe_bWantsToSprint
		&& AdvancedCharacter->CanSprint(); // We should not sprint already
}

void UAdvancedMovementComponent::EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
	bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, nullptr);
}

void UAdvancedMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
	if (Slide_ResetVelocity)
	{
		Velocity = FVector::ZeroVector;
	}
}

bool UAdvancedMovementComponent::CanSlide() const
{
	const FVector start = UpdatedComponent->GetComponentLocation();
	const FVector end = start +
		CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.5f * FVector::DownVector;
	static FName ProfileName = TEXT("BlockAll");
	const bool bValidSurface = GetWorld()->LineTraceTestByProfile(start, end, ProfileName,
	                                                              AdvancedCharacter->GetIgnoreCharacterParams());
	const bool bEnoughSpeed = Velocity.SizeSquared() > pow(Slide_MinSpeed, 2);

	return bValidSurface && bEnoughSpeed;
}


void UAdvancedMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	// No physics here
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CanSlide())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	// For root mootion
	RestorePreAdditiveRootMotionVelocity();

	// Check for ground under us and minimum velocity
	FHitResult surfaceHit;
	if (!GetSlideSurface(surfaceHit)
		|| Velocity.SizeSquared() < FMath::Pow(Slide_MinSpeed, 2))
	{
		ExitSlide();
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	// Surface gravity
	Velocity += Slide_GravityForce * FVector::DownVector * DeltaTime;

	// Strafe
	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5)
	{
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}
	else
	{
		Acceleration = FVector::ZeroVector;
	}

	// Calc Velocity
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasAdditiveVelocity())
	{
		CalcVelocity(DeltaTime, Slide_Friction, false, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(DeltaTime);

	// Perform move
	Iterations++;
	bJustTeleported = false;

	FVector oldLoc = UpdatedComponent->GetComponentLocation();
	FQuat oldRot = UpdatedComponent->GetComponentRotation().Quaternion();

	FHitResult hit(1.f);
	FVector adjusted = Velocity * DeltaTime; // x = v * at
	FVector velPlaneDir = FVector::VectorPlaneProject(Velocity, surfaceHit.Normal).GetSafeNormal();
	FQuat newRot = FRotationMatrix::MakeFromXZ(velPlaneDir, surfaceHit.Normal).ToQuat();
	SafeMoveUpdatedComponent(adjusted, newRot, true, hit);

	// If we hit wall
	if (hit.Time < 1.f)
	{
		HandleImpact(hit, DeltaTime, adjusted);
		SlideAlongSurface(adjusted, (1.f - hit.Time), hit.Normal, hit, true);
	}

	FHitResult newSurfaceHit;
	if (!GetSlideSurface(newSurfaceHit)
		|| Velocity.SizeSquared() < FMath::Pow(Slide_MinSpeed, 2))
	{
		ExitSlide();
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasAdditiveVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - oldLoc) / DeltaTime; // v = dx/dt
	}
}


/*void UXeusMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CanSlide())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * Slide_GravityForce * deltaTime;

		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector().GetSafeNormal2D());

		// Apply acceleration
		CalcVelocity(timeTick, GroundFriction * Slide_Friction, false, GetMaxBrakingDeceleration());

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f, 0.f, -1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if (!NewDelta.IsZero())
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// Make velocity reflect actual move
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}*/

bool UAdvancedMovementComponent::GetSlideSurface(FHitResult& Hit) const
{
	FVector start = UpdatedComponent->GetComponentLocation();
	FVector end = start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f *
		FVector::DownVector;
	static FName profileName = TEXT("BlockAll");
	return GetWorld()->LineTraceSingleByProfile(Hit, start, end, profileName,
	                                            AdvancedCharacter->GetIgnoreCharacterParams());
}


bool UAdvancedMovementComponent::IsSliding() const
{
	return IsCustomMovementMode(ECustomMovementMode::CMOVE_Slide);
}

void UAdvancedMovementComponent::SprintPressed()
{
	if (IsSprintingAllowed())
	{
		Safe_bWantsToSprint = true;
	}
}

void UAdvancedMovementComponent::SprintReleased()
{
	Safe_bWantsToSprint = false;
}

void UAdvancedMovementComponent::CrouchPressed()
{
	bWantsToCrouch = !bWantsToCrouch;
	// GetWorld()->GetTimerManager()
	//           .SetTimer(TimerHandle_EnterProne, this,
	//                     &UXeusMovementComponent::TryEnterProne, Prone_EnterHoldDuration);
}

void UAdvancedMovementComponent::CrouchReleased()
{
	// GetWorld()->GetTimerManager()
	//           .ClearTimer(TimerHandle_EnterProne);
}

void UAdvancedMovementComponent::SlidePressed()
{
	Safe_bWantsToSlide = true;
}

void UAdvancedMovementComponent::SlideReleased()
{
	Safe_bWantsToSlide = false;
}

void UAdvancedMovementComponent::DashPressed()
{
	const float currentTime = GetWorld()->GetTimeSeconds();
	if (IsAbleToDash())
	{
		Safe_bWantsToDash = true;
	}
	else
	{
		GetWorld()->GetTimerManager().
		            SetTimer(TimerHandle_DashCooldown, this,
		                     &UAdvancedMovementComponent::OnDashCooldownFinished,
		                     Dash_CooldownDuration - (currentTime - DashStartTime));
	}
}

bool UAdvancedMovementComponent::IsAbleToDash() const
{
	const float currentTime = GetWorld()->GetTimeSeconds();
	return currentTime - DashStartTime >= Dash_CooldownDuration;
}

void UAdvancedMovementComponent::DashReleased()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_DashCooldown);
}

bool UAdvancedMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UAdvancedMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

bool UAdvancedMovementComponent::CanDash() const
{
	return IsWalking() &&
		!IsSprinting() &&
		!IsSliding() &&
		!IsCrouching() &&
		!IsFalling() && AdvancedCharacter->CanDash();
}

void UAdvancedMovementComponent::PerformDash()
{
	DashStartTime = GetWorld()->TimeSeconds;
	FVector dashDir = (Acceleration.IsNearlyZero() ? UpdatedComponent->GetForwardVector() : Acceleration).
		GetSafeNormal2D();
	dashDir += FVector::UpVector * .1f;
	const EDashDirection dashSide = CalculateDashDirection(CalculateDirection());
	float dash_impulse;
	switch (dashSide)
	{
	case DASH_Left:
		dash_impulse = Dash_Impulse_L;
		break;
	case DASH_Right:
		dash_impulse = Dash_Impulse_R;
		break;
	case DASH_Forward:
		dash_impulse = Dash_Impulse_F;
		break;
	case DASH_Backward:
		dash_impulse = Dash_Impulse_B;
		break;
	default:
		dash_impulse = Dash_Impulse_F;
	}

	Velocity = dash_impulse * dashDir;

	const FQuat newRot = FRotationMatrix::MakeFromXZ(dashDir, FVector::UpVector).ToQuat();
	FHitResult hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, newRot, false, hit);

	SetMovementMode(MOVE_Falling);

	OnDashStarted.Broadcast(this, dashSide);
}

void UAdvancedMovementComponent::OnDashCooldownFinished()
{
	Safe_bWantsToDash = true;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
UAdvancedMovementComponent::EDashDirection UAdvancedMovementComponent::CalculateDashDirection(float InAngle) const
{
	if (InAngle <= -150.0f || InAngle >= 150.0f)
	{
		return EDashDirection::DASH_Backward;
	}
	if (InAngle >= -50.0f && InAngle <= 50.0f)
	{
		return EDashDirection::DASH_Forward;
	}
	if (InAngle <= -50.0f)
	{
		return EDashDirection::DASH_Left;
	}
	if (InAngle >= 50.0f)
	{
		return EDashDirection::DASH_Right;
	}
	return EDashDirection::DASH_Forward;
}

float UAdvancedMovementComponent::CalculateDirection() const
{
	if (!IsValid(AdvancedCharacter))
		return 0.0f;
	const FMatrix rotMatrix = FRotationMatrix(AdvancedCharacter->GetControlRotation());
	const FVector forwardVector = rotMatrix.GetScaledAxis(EAxis::X);
	const FVector fightVector = rotMatrix.GetScaledAxis(EAxis::Y);
	const FVector normalizedVel = Velocity.GetSafeNormal2D();

	// get a cos(alpha) of forward vector vs velocity
	const float forwardCosAngle = FVector::DotProduct(forwardVector, normalizedVel);
	// now get the alpha and convert to degree
	float forwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(forwardCosAngle));

	// depending on where right vector is, flip it
	const float rightCosAngle = FVector::DotProduct(fightVector, normalizedVel);
	if (rightCosAngle < 0)
	{
		forwardDeltaDegree *= -1;
	}

	return forwardDeltaDegree;
}

void UAdvancedMovementComponent::OnRep_DashStart()
{
	OnDashStarted.Broadcast(this, CalculateDashDirection(CalculateDirection()));
}
