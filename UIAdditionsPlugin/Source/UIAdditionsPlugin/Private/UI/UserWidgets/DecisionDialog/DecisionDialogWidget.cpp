/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "DecisionDialogWidget.h"
#include "ButtonWidget.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"
#include "Animation/WidgetAnimation.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "Misc/FrameNumber.h"
#include "MovieScene.h"
#include "LogUIAdditionsPlugin.h"


// Setup

UDecisionDialogWidget::UDecisionDialogWidget(const FObjectInitializer& InObjectInitializer)
	: Super(InObjectInitializer) {

	AdditionalInputActionReplyMode = E_AdditionalInputActionReplyModes::HandleAllExceptSpecified;
}

void UDecisionDialogWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	// Bind the buttons
	if (IsValid(ButtonYes)) {
		ButtonYes->OnButtonReleased.AddDynamic(this, &UDecisionDialogWidget::ActOnButtonYesActivated);
	}
	if (IsValid(ButtonNo)) {
		ButtonNo->OnButtonReleased.AddDynamic(this, &UDecisionDialogWidget::ActOnButtonNoActivated);
	}

	// Create an animation, which will be used simply to loop a second.
	AnimAutoDecide = NewObject<UWidgetAnimation>(this, TEXT("AnimAutoDecide"));
	if (IsValid(AnimAutoDecide)) {
		AnimAutoDecide->MovieScene = NewObject<UMovieScene>(this, FName("AnimAutoDecideScene"));
		if (IsValid(AnimAutoDecide->MovieScene)) {
			// 1 second.
			AnimAutoDecide->MovieScene->SetPlaybackRange(TRange<FFrameNumber>(0, 60000));
			//AnimAutoDecide->MovieScene->SetPlaybackRangeLocked(true);
		}

		// Bind to the animation which will auto decide when it finishes.
		OnWidgetAnimAutoDecideFinished.BindDynamic(this, &UDecisionDialogWidget::ActOnWidgetAnimAutoDecideFinished);
		BindToAnimationFinished(AnimAutoDecide, OnWidgetAnimAutoDecideFinished);
	}
}

void UDecisionDialogWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	// Set up defaults
	AutoDecisionRemainingDuration = AutoDecisionDuration;

	// Set up the widget text.
	if (IsValid(TitleTextWidget)) {
		TitleTextWidget->SetText(Title);	
	}

	if (IsValid(DescriptionWidget)) {
		DescriptionWidget->SetText(Description);	
	}

	if (IsValid(AutoDecideDescriptionTextWidget)) {
		AutoDecideDescriptionTextWidget->SetText(AutoDecideDescription);	
	}

	UpdateAutoDecideDurationWidget();

	if (IsValid(AutoDecideDescriptionPanelWidget)) {
		AutoDecideDescriptionPanelWidget->SetVisibility(bEnableAutoDecide ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

// Navigation

void UDecisionDialogWidget::RestartAutoDecisionAnimation() {
	// Reset remaining duration and start playing the AutoDecide "delay" if desired.
	AutoDecisionRemainingDuration = AutoDecisionDuration;

	// update auto decision text (counter)
	UpdateAutoDecideDurationWidget();

	if (bEnableAutoDecide) {
		PlayAnimation(AnimAutoDecide);
	}
}

void UDecisionDialogWidget::NativeShow() {
	Super::NativeShow();

	RestartAutoDecisionAnimation();
}

void UDecisionDialogWidget::NativeHide() {
	Super::NativeHide();

	StopAnimation(AnimAutoDecide);
}

// AutoDecide

int32 UDecisionDialogWidget::GetAutoDecisionDuration() const {
	return AutoDecisionDuration;
}

int32 UDecisionDialogWidget::GetAutoDecisionRemainingDuration() const {
	return AutoDecisionRemainingDuration;
}

bool UDecisionDialogWidget::GetEnableAutoDecide() const {
	return bEnableAutoDecide;
}

void UDecisionDialogWidget::UpdateAutoDecideDurationWidget_Implementation() {
	if (IsValid(AutoDecideDurationTextWidget)) {
		AutoDecideDurationTextWidget->SetText(UKismetTextLibrary::Conv_IntToText(GetAutoDecisionRemainingDuration()));
	}
}

// Delegates | DecisionDialogButton

void UDecisionDialogWidget::ActOnButtonYesActivated(UButtonWidget* InButtonWidget) { 
	OnDecided.Broadcast(E_DecisionDialogDecisions::Yes);
	RequestUINavigation(NAME_None);
}

void UDecisionDialogWidget::ActOnButtonNoActivated(UButtonWidget* InButtonWidget) { 
	OnDecided.Broadcast(E_DecisionDialogDecisions::No);
	RequestUINavigation(NAME_None);
}

// Delegates | Timer

void UDecisionDialogWidget::ActOnWidgetAnimAutoDecideFinished() {
	const float SpentDuration = FMath::Abs(AnimAutoDecide->GetStartTime() - AnimAutoDecide->GetEndTime());
	AutoDecisionRemainingDuration -= SpentDuration;
	AutoDecisionRemainingDuration = FMath::Max(AutoDecisionRemainingDuration, 0);

	// update auto decision text (counter)
	UpdateAutoDecideDurationWidget();

	// Loop animation or auto decide
	if (AutoDecisionRemainingDuration == 0) {
		switch(AutoDecision) {
		case (E_DecisionDialogDecisions::Yes):
			ActOnButtonYesActivated(ButtonYes);
			break;
		case (E_DecisionDialogDecisions::No):
			ActOnButtonNoActivated(ButtonNo);
			break;
		}
	}
	else {
		PlayAnimation(AnimAutoDecide);
	}
}
