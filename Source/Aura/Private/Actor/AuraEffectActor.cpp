

#include "Actor/AuraEffectActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystemBlueprintLibrary.h"



AAuraEffectActor::AAuraEffectActor()
{

	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();

}

void AAuraEffectActor::ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	//IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);
	//if (ASCInterface)
	//{
	//	ASCInterface->GetAbilitySystemComponent();
	//	UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	//}
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (TargetASC == nullptr) return;
	check(GameplayEffectClass)

	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffecSpectHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass,1.0f, EffectContextHandle);
	FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffecSpectHandle.Data.Get());

	const bool bIsInfinite = EffecSpectHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}

}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstantEffectAplicationPolicy == EEffectAplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectAplicationPolicy == EEffectAplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectAplicationPolicy == EEffectAplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}

}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (InstantEffectAplicationPolicy == EEffectAplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectAplicationPolicy == EEffectAplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC)) return;
		TArray<FActiveGameplayEffectHandle> HandlesToRemove;

		for (auto HandlePair : ActiveEffectHandles)
		{
			if (TargetASC == HandlePair.Value)
			{
				TargetASC->RemoveActiveGameplayEffect(HandlePair.Key,1);
				HandlesToRemove.Add(HandlePair.Key);
			}
		}

		for (auto& handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(handle);
		}
	}

}
