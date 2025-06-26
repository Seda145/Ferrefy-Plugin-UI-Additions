/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "LazyWidget.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Blueprint/UserWidget.h"
#include "SLazyWidget.h"
#include "LazyWidgetSlot.h"
#include "ObjectEditorUtils.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "SlateUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LogUIAdditionsPlugin.h"

#define LOCTEXT_NAMESPACE "UIAdditionsPlugin"


// Setup

ULazyWidget::ULazyWidget(const FObjectInitializer& InObjectInitializer)
	: Super(InObjectInitializer) {
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void ULazyWidget::SynchronizeProperties() {
	Super::SynchronizeProperties();
	if (MyLazyWidget.IsValid()) {
		MyLazyWidget->SetHAlign(HorizontalAlignment);
		MyLazyWidget->SetVAlign(VerticalAlignment);
		MyLazyWidget->SetPadding(Padding);
	}
}

void ULazyWidget::ReleaseSlateResources(bool bInReleaseChildren) {
	Super::ReleaseSlateResources(bInReleaseChildren);

	MyLazyWidget.Reset();
}

void ULazyWidget::PostLoad() {
	Super::PostLoad();
	if (GetChildrenCount() > 0) {
		if (UPanelSlot* PanelSlot = GetContentSlot()) {
			ULazyWidgetSlot* LazyWidgetSlot = Cast<ULazyWidgetSlot>(PanelSlot);
			if (LazyWidgetSlot == nullptr) {
				LazyWidgetSlot = NewObject<ULazyWidgetSlot>(this);
				LazyWidgetSlot->Content = GetContent();
				LazyWidgetSlot->Content->Slot = LazyWidgetSlot;
				Slots[0] = LazyWidgetSlot;
			}
		}
	}
}

UClass* ULazyWidget::GetSlotClass() const {
	return ULazyWidgetSlot::StaticClass();
}

void ULazyWidget::OnSlotAdded(UPanelSlot* InSlot) {
	// Copy the content properties into the new slot so that it matches what has been setup
	// so far by the user.
	ULazyWidgetSlot* LazyWidgetSlot = CastChecked<ULazyWidgetSlot>(InSlot);
	LazyWidgetSlot->Padding = Padding;
	LazyWidgetSlot->HorizontalAlignment = HorizontalAlignment;
	LazyWidgetSlot->VerticalAlignment = VerticalAlignment;

	// Add the child to the live slot if it already exists
	if (MyLazyWidget.IsValid()) {
		// Construct the underlying slot.
		LazyWidgetSlot->BuildSlot(MyLazyWidget.ToSharedRef());
	}
}

void ULazyWidget::OnSlotRemoved(UPanelSlot* InSlot) {
	// Remove the widget from the live slot if it exists.
	if (MyLazyWidget.IsValid()) {
		MyLazyWidget->SetContent(SNullWidget::NullWidget);
	}
}

TSharedRef<SWidget> ULazyWidget::RebuildWidget() {
	MyLazyWidget = SNew(SLazyWidget);

	if (GetChildrenCount() > 0) 	{
		Cast<ULazyWidgetSlot>(GetContentSlot())->BuildSlot(MyLazyWidget.ToSharedRef());
	}

	return MyLazyWidget.ToSharedRef();
}

void ULazyWidget::OnWidgetRebuilt() {
	Super::OnWidgetRebuilt();

#if WITH_EDITORONLY_DATA

	if (IsDesignTime()) {
		if (!bLoadInEditor) {
			return;
		}
		bool bOriginalAsync = GetLoadAsync();
		SetLoadAsync(false);
		// Disable async for editor load.
		LoadContent();
		// Then restore.
		SetLoadAsync(bOriginalAsync);

		// Nothing else to do.
		return;
	}

#endif // WITH_EDITORONLY_DATA

	/* OnWidgetRebuilt should be called whenever this widget is added to a parent (Construct).
	* Bind to when the outer (UUserWidget) becomes visible and then attempt to automatically lazy load.
	* There is currently no engine way to accurately get the actual visibility of a UWidget or SWidget relative to its ancestors.
	* This is the simplest way to implement basic functionality, to respond to a change in the visibility delegate on an outer UUserWidget.
	*/

	if (IsValid(OuterUserWidget)) {
		OuterUserWidget->OnVisibilityChanged.RemoveDynamic(this, &ULazyWidget::ActOnOuterVisibilityChanged);
	}
	OuterUserWidget = GetTypedOuter<UUserWidget>();
	if (IsValid(OuterUserWidget)) {
		OuterUserWidget->OnVisibilityChanged.AddDynamic(this, &ULazyWidget::ActOnOuterVisibilityChanged);
	}

	ConditionalLoadOrUnloadContent();
}

#if WITH_EDITOR

void ULazyWidget::PostEditChangeProperty(struct FPropertyChangedEvent& InPropertyChangedEvent) {
	Super::PostEditChangeProperty(InPropertyChangedEvent);

	static bool IsReentrant = false;

	if (!IsReentrant) {
		IsReentrant = true;

		if (InPropertyChangedEvent.Property) {
			static const FName PaddingName("Padding");
			static const FName HorizontalAlignmentName("HorizontalAlignment");
			static const FName VerticalAlignmentName("VerticalAlignment");

			if (ULazyWidgetSlot* LazyWidgetSlot = Cast<ULazyWidgetSlot>(GetContentSlot())) {
				if (InPropertyChangedEvent.Property->GetFName() == PaddingName) {
					FObjectEditorUtils::MigratePropertyValue(this, PaddingName, LazyWidgetSlot, PaddingName);
				}
				else if (InPropertyChangedEvent.Property->GetFName() == HorizontalAlignmentName) {
					FObjectEditorUtils::MigratePropertyValue(this, HorizontalAlignmentName, LazyWidgetSlot, HorizontalAlignmentName);
				}
				else if (InPropertyChangedEvent.Property->GetFName() == VerticalAlignmentName) {
					FObjectEditorUtils::MigratePropertyValue(this, VerticalAlignmentName, LazyWidgetSlot, VerticalAlignmentName);
				}
				else if (InPropertyChangedEvent.Property->GetFName() == TEXT("bLoadInEditor")) {
					if (bLoadInEditor) {
						bool bOriginalAsync = GetLoadAsync();
						SetLoadAsync(false);
						// Disable async for editor load.
						LoadContent();
						// Then restore.
						SetLoadAsync(bOriginalAsync);
					}
					else {
						UnloadContent();
					}
				}
			}
		}

		IsReentrant = false;
	}
}

// Palette

const FText ULazyWidget::GetPaletteCategory() {
	return LOCTEXT("Advanced", "Advanced");
}

#endif

// Appearance

void ULazyWidget::SetVisibility(ESlateVisibility InVisibility) {
	Super::SetVisibility(InVisibility);
	ConditionalLoadOrUnloadContent();
}

EHorizontalAlignment ULazyWidget::GetHorizontalAlignment() const {
	return HorizontalAlignment;
}

void ULazyWidget::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment) {
	HorizontalAlignment = InHorizontalAlignment;
	if (MyLazyWidget.IsValid()) {
		MyLazyWidget->SetHAlign(GetHorizontalAlignment());
	}
}

EVerticalAlignment ULazyWidget::GetVerticalAlignment() const {
	return VerticalAlignment;
}

void ULazyWidget::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment) {
	VerticalAlignment = InVerticalAlignment;
	if (MyLazyWidget.IsValid()) {
		MyLazyWidget->SetVAlign(GetVerticalAlignment());
	}
}

FMargin ULazyWidget::GetPadding() const {
	return Padding;
}

void ULazyWidget::SetPadding(FMargin InPadding) {
	Padding = InPadding;
	if (MyLazyWidget.IsValid()) {
		MyLazyWidget->SetPadding(GetPadding());
	}
}

// Lazy

bool ULazyWidget::GetLoadAsync() const {
	return bLoadAsync;
}

void ULazyWidget::SetLoadAsync(bool bInLoadAsync) {
	bLoadAsync = bInLoadAsync;
}

bool ULazyWidget::GetLoadControlledByOuterVisibility() const {
	return bLoadControlledByOuterVisibility;
}

void ULazyWidget::SetLoadControlledByOuterVisibility(bool bInLoadControlledByOuterVisibility) {
	if (GetLoadControlledByOuterVisibility() == bInLoadControlledByOuterVisibility) {
		return;
	}

	bLoadControlledByOuterVisibility = bInLoadControlledByOuterVisibility;
	ConditionalLoadOrUnloadContent();
}

bool ULazyWidget::GetLoadControlledBySelfVisibility() const {
	return bLoadControlledBySelfVisibility;
}

void ULazyWidget::SetLoadControlledBySelfVisibility(bool bInLoadControlledBySelfVisibility) {
	if (GetLoadControlledBySelfVisibility() == bInLoadControlledBySelfVisibility) {
		return;
	}

	bLoadControlledBySelfVisibility = bInLoadControlledBySelfVisibility;
	ConditionalLoadOrUnloadContent();
}

bool ULazyWidget::GetCollectGarbageOnUnload() const {
	return bCollectGarbageOnUnload;
}

void ULazyWidget::SetCollectGarbageOnUnload(bool bInCollectGarbageOnUnload) {
	bCollectGarbageOnUnload = bInCollectGarbageOnUnload;
}

TSoftClassPtr<UUserWidget> ULazyWidget::GetLazyContent() const {
	return LazyContent;
}

void ULazyWidget::SetLazyContent(const TSoftClassPtr<UUserWidget> InLazyContent) {
	LazyContent = InLazyContent;
}

void ULazyWidget::UnloadContent() {
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Unloading content"));
	// Clear streaming so we are sure not to load new content through it.
	if (StreamingHandle.IsValid()) {
		StreamingHandle->CancelHandle();
		// Any delegates to the handle should be unbound automatically. Nothing left to do here.
		StreamingHandle.Reset();
	}
	// This will clear the slot, then call OnSlotRemoved to sync the slate widget.
	ClearChildren();

	if (GetCollectGarbageOnUnload()) {

#if WITH_EDITOR

		if (IsDesignTime()) {
			// or the log will fill with "Compacting FUObjectHashTables". Editor doesn't like it.
			return;
		}

#endif // WITH_EDITOR

		/* 
		* Invisible and even removed widgets can still cause a terrible performance in the Slate system until they are garbage collected.
		* Tip: The hit of collecting garbage might be unnoticable if you had pause on.
		*/
		UKismetSystemLibrary::CollectGarbage();
	}
}

void ULazyWidget::ConditionalLoadOrUnloadContent() {
	if (GetLazyContent().IsNull()) {
		return;
	}
	if (!GetLoadControlledBySelfVisibility()
		&& !GetLoadControlledByOuterVisibility()
		) {
		// By default do nothing if there is no condition to control the loading.
		return;
	}

	if (GetLoadControlledBySelfVisibility() && !USlateUtils::IsVisible(GetVisibility())) {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Unload, self is not visible."), *GetName());
		UnloadContent();
		return;
	}

	if (GetLoadControlledByOuterVisibility()) {
		if (!IsValid(OuterUserWidget) || !USlateUtils::IsVisible(OuterUserWidget->GetVisibility())) {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Unload, OuterUserWidget is not visible."), *GetName());
			UnloadContent();
			return;
		}
	} 

	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: LoadContent."), *GetName());
	LoadContent();
}

void ULazyWidget::SetLoadedContent(const TSubclassOf<UUserWidget> InContentClass) {
	// This will set the slot, then call OnSlotAdded to sync with the slate widget.
	UUserWidget* NewContent = CreateWidget(this, InContentClass);
	if (IsValid(NewContent)) {
		SetContent(NewContent);
		// Note that SetContent does not immediately result in Initialize / (Pre) Construct being called on a widget.
		OnLoadComplete.Broadcast();
	}
}

void ULazyWidget::LoadContent() {
	if (GetLazyContent().IsNull()) {
		return;
	}
	if (IsValid(GetContent()) && GetContent()->GetClass() == GetLazyContent().Get()) {
		// Don't continue if we have already loaded and set the content.
		OnLoadComplete.Broadcast();
		return;
	}
	if (StreamingHandle.IsValid()) {
		TArray<FSoftObjectPath> OutAssetList;
		StreamingHandle->GetRequestedAssets(OutAssetList, false);
		if (OutAssetList.Num() > 0 && OutAssetList[0] == GetLazyContent().ToSoftObjectPath()) {
			// Don't continue if currently loading the content.
			return;
		}
	}

	UnloadContent();

	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Loading content"));

	if (!GetLoadAsync()) {
		SetLoadedContent(GetLazyContent().LoadSynchronous());
		// Finished.
		return;
	}

	if (GetLazyContent().Get()) {
		// Already loaded, but not set.
		SetLoadedContent(GetLazyContent().Get());
		// Finished.
		return;
	}

	// Else start lazy load.

	OnLazyLoadStarted.Broadcast();

	TWeakObjectPtr<ULazyWidget> WeakThis(this);
	StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(GetLazyContent().ToSoftObjectPath(),[WeakThis]() {
		ULazyWidget* StrongThis = WeakThis.Get();
		if (IsValid(StrongThis)) {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: lazy CompleteDelegate"), *StrongThis->GetName());
			StrongThis->SetLoadedContent(StrongThis->GetLazyContent().Get());
		}
	}, FStreamableManager::AsyncLoadHighPriority);

	/* 
	* Bind to the handle delegates so we can broadcast its change in state.
	* Binding can only be done when loading is in progress.
	* This is useful for UUserWidget where we want to respond visually with a loading bar, text, spinner etc.
	* We don't hardcode any of that here, it's up to the graphics designer.
	*/
	if (StreamingHandle.IsValid()) {

		// On cancelled

		FStreamableDelegate CancelDelegate;
		CancelDelegate.BindLambda([WeakThis]() {
			ULazyWidget* StrongThis = WeakThis.Get();
			if (IsValid(StrongThis)) {
				UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: lazy CancelDelegate"), *StrongThis->GetName());
				StrongThis->OnLazyLoadCancelled.Broadcast();
			}
		});
		StreamingHandle->BindCancelDelegate(CancelDelegate);

		// On updated

		FStreamableUpdateDelegate UpdateDelegate;
		UpdateDelegate.BindLambda([WeakThis](TSharedRef<FStreamableHandle> InHandle) {
			ULazyWidget* StrongThis = WeakThis.Get();
			if (IsValid(StrongThis)) {
				UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: lazy UpdateDelegate"), *StrongThis->GetName());
				StrongThis->OnLazyLoadUpdates.Broadcast();
			}
		});
		StreamingHandle->BindUpdateDelegate(UpdateDelegate);
	}
}

// Delegates

void ULazyWidget::ActOnOuterVisibilityChanged(ESlateVisibility InVisibility) {
	ConditionalLoadOrUnloadContent();
}


#undef LOCTEXT_NAMESPACE
