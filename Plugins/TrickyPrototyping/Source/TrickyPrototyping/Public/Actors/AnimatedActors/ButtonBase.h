// Copyright (c) 2021 Artyom "Tricky Fat Cat" Volkov (tricky.fat.cat@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "Actors/AnimatedActor.h"
#include "Interfaces/InteractionInterface.h"
#include "ButtonBase.generated.h"

class UInteractionSphereComponent;
class USceneComponent;

/**
 * A base button class. Use it for creating different buttons and levers
 */

UENUM(BlueprintType)
enum class EButtonBehaviour : uint8
{
	Key,
	Switch
};

UCLASS()
class TRICKYPROTOTYPING_API AButtonBase : public AAnimatedActor, public IInteractionInterface
{
	GENERATED_BODY()

public:
	AButtonBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void Disable() override;
	virtual void Enable() override;

protected:
	virtual void FinishAnimation() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Components")
	USceneComponent* ButtonRoot = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Components")
	UInteractionSphereComponent* ButtonTrigger = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Button")
	EButtonBehaviour ButtonBehaviour = EButtonBehaviour::Switch;

private:
	UPROPERTY(EditAnywhere,
		BlueprintReadWrite,
		Category="Button",
		meta=(AllowPrivateAccess="true", EditCondition="ButtonBehaviour == EButtonBehaviour::Key", ClampMin="0"))
	float KeyAutoCloseDelayDuration = 1.f;

	UPROPERTY(EditAnywhere, Category="Button")
	bool bRequireInteraction = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Button", meta=(AllowPrivateAccess="true", EditCondition="bRequireInteraction"))
	bool bRequireLineOfSight = false;

	UPROPERTY(EditAnywhere,
		Category="Button",
		meta=(AllowPrivateAccess="true", EditCondition="!bRequireInteraction", ClampMin="0"))
	float BeginOverlapDelay = 1.f;

	UPROPERTY(EditAnywhere,
		Category="Button",
		meta=(AllowPrivateAccess="true", EditCondition="!bRequireInteraction", ClampMin="0"))
	float EndOverlapDelay = 1.f;

	FTimerHandle BeginOverlapDelayHandle;

	FTimerHandle EndOverlapDelayHandle;

	FTimerHandle AutoCloseDelayHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button", meta=(AllowPrivateAccess="true"))
	bool bPressOnce = false;

	virtual bool ProcessInteraction_Implementation(AActor* TargetActor) override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	                           AActor* OtherActor,
	                           UPrimitiveComponent* OtherComp,
	                           int32 OtherBodyIndex,
	                           bool bFromSweep,
	                           const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent,
	                         AActor* OtherActor,
	                         UPrimitiveComponent* OtherComp,
	                         int32 OtherBodyIndex);

	void ProcessTriggerOverlap();
};
