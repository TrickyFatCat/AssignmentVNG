// Copyright (c) 2021 Artyom "Tricky Fat Cat" Volkov (tricky.fat.cat@gmail.com)


#include "Actors/AnimatedActors/DoorBase.h"

#include "Components/KeyRingComponent.h"
#include "Components/TriggerComponents/InteractionBoxComponent.h"

ADoorBase::ADoorBase()
{
	DoorRoot = CreateDefaultSubobject<USceneComponent>("DoorRoot");
	SetRootComponent(DoorRoot);

	DoorTrigger = CreateDefaultSubobject<UInteractionBoxComponent>("DoorTrigger");
	DoorTrigger->SetupAttachment(GetRootComponent());
}

void ADoorBase::BeginPlay()
{
	Super::BeginPlay();
	SwitchDoorType();

	DoorTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADoorBase::OnTriggerBeginOverlap);
	DoorTrigger->OnComponentEndOverlap.AddDynamic(this, &ADoorBase::OnTriggerEndOverlap);
}

void ADoorBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ADoorBase::Disable()
{
	Super::Disable();

	DoorTrigger->SetIsEnabled(false);
}

void ADoorBase::Enable()
{
	Super::Enable();

	SwitchDoorType();
}

void ADoorBase::FinishAnimation()
{
	Super::FinishAnimation();

	switch (GetStateCurrent())
	{
	case EAnimatedActorState::Opened:
		if (IsClosingAutomatically() && !DoorTrigger->GetIsActorInside())
		{
			StartAutoClose();
			return;
		}

		if (!IsClosingAutomatically() && !DoorTrigger->GetIsActorInside())
		{
			Close();
			return;
		}
		break;

	case EAnimatedActorState::Closed:
		if (DoorTrigger->GetIsActorInside() && DoorType == EDoorType::Auto)
		{
			Open();
		}
		break;
	}
}

void ADoorBase::SetDoorType(const EDoorType Value)
{
	if (DoorType == Value) return;

	DoorType = Value;
	SwitchDoorType();
}

void ADoorBase::SwitchDoorType() const
{
	switch (DoorType)
	{
	case EDoorType::Auto:
		DoorTrigger->SetIsNormalTrigger(true);
		break;

	case EDoorType::Interactive:
		DoorTrigger->SetIsNormalTrigger(false);
		DoorTrigger->bRequireLineOfSight = bRequireLineOfSight;
		break;

	case EDoorType::Manual:
		DoorTrigger->SetIsEnabled(false);
		break;
	}
}

void ADoorBase::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                      AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp,
                                      int32 OtherBodyIndex,
                                      bool bFromSweep,
                                      const FHitResult& SweepResult)
{
	if (IsClosingAutomatically() && IsStateCurrent(EAnimatedActorState::Opened))
	{
		StopAutoClose();
		return;
	}

	switch (DoorType)
	{
	case EDoorType::Auto:
		if (bRequireKey && !HasKey(OtherActor)) return;

		if (GetIsReversible())
		{
			Open();
			return;
		}

		if (IsStateCurrent(EAnimatedActorState::Closed))
		{
			Open();
		}
		else if (IsStateCurrent(EAnimatedActorState::Opened))
		{
			Close();
		}

		break;

	case EDoorType::Interactive:
		break;
	}
}

void ADoorBase::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                    AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp,
                                    int32 OtherBodyIndex)
{
	if (IsClosingAutomatically() && IsStateCurrent(EAnimatedActorState::Opened))
	{
		StartAutoClose();
		return;
	}

	switch (DoorType)
	{
	case EDoorType::Auto:
		if (GetStateCurrent() != EAnimatedActorState::Opened && !GetIsReversible()) return;

		Close();
		break;

	case EDoorType::Interactive:
		break;
	}
}

bool ADoorBase::ProcessInteraction_Implementation(AActor* TargetActor)
{
	if (!TargetActor || DoorType != EDoorType::Interactive) return false;

	if (bRequireKey && !HasKey(TargetActor)) return false;

	if (GetIsReversible() && IsStateCurrent(EAnimatedActorState::Transition))
	{
		switch (GetStateTarget())
		{
		case EAnimatedActorState::Opened:
			Close();
			break;

		case EAnimatedActorState::Closed:
			Open();
			break;
		}

		return true;
	}

	StopAutoClose();

	switch (GetStateCurrent())
	{
	case EAnimatedActorState::Opened:
		Close();
		break;

	case EAnimatedActorState::Closed:
		Open();
		break;
	}

	return true;
}

bool ADoorBase::IsClosingAutomatically() const
{
	return AutoCloseDelay > 0.f;
}

void ADoorBase::StartAutoClose()
{
	if (!IsStateCurrent(EAnimatedActorState::Opened)) return;

	GetWorldTimerManager().SetTimer(AutoCloseDelayHandle, this, &ADoorBase::Close, AutoCloseDelay, false);
}

void ADoorBase::StopAutoClose()
{
	if (GetWorldTimerManager().IsTimerActive(AutoCloseDelayHandle))
	{
		GetWorldTimerManager().ClearTimer(AutoCloseDelayHandle);
	}
}

bool ADoorBase::HasKey(const AActor* Actor) const
{
	if (!IsValid(Actor)) return false;

	UKeyRingComponent* KeyRingComponent = Actor->FindComponentByClass<UKeyRingComponent>();

	if (!KeyRingComponent) return false;

	return KeyRingComponent->HasKey(DoorKey);
}
