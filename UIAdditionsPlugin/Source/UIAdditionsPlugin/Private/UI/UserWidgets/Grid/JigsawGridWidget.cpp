/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "JigsawGridWidget.h"
#include "LogUIAdditionsPlugin.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"


// Setup

void UJigsawGridWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	if (IsValid(GridSizeBoxWidget)) {
		GridSizeBoxWidget->SetMinDesiredWidth(ColumnsPerRow * SlotPixelSize);
	}
}

// Grid

TArray<FIntPoint> UJigsawGridWidget::GetTrackedFreeSpace() const {
	return TrackedFreeSpace;
}

TMap<UWidget*, FS_JigsawGridSlotInfo> UJigsawGridWidget::GetSlots() const {
	return GridSlots;
}

void UJigsawGridWidget::ClearGrid() {
	if (!IsValid(GridCanvasWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InWidget GridCanvasWidget"));
	}
	else {
		GridCanvasWidget->ClearChildren();
	}

	GridSlots.Empty();
	TrackedFreeSpace.Empty();
	
	// Initialize the first row as "free" to simplify later calculations.
	GridTrackedYBound = 0;
	for (int i = 0; i < ColumnsPerRow; i++) {
		TrackedFreeSpace.Add(FIntPoint(i, GridTrackedYBound));
	}
}

bool UJigsawGridWidget::GetSlotInfoBySlotWidget(UWidget* InWidget, FS_JigsawGridSlotInfo& OutSlotInfo) const {
	OutSlotInfo = FS_JigsawGridSlotInfo();
	if (!IsValid(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InWidget invalid"));
		return false;
	}

	const FS_JigsawGridSlotInfo* SlotInfo = GetSlots().Find(InWidget);
	if (SlotInfo) {
		OutSlotInfo = *SlotInfo;
		return true;
	}
	return false;
}

bool UJigsawGridWidget::CanPlaceSlotAtPosition(FIntPoint InPosition, FIntPoint InSlotSize) const {
	if (InSlotSize.X < 1 || InSlotSize.Y < 1) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InSlotSize is smaller than 1."));
		return false;
	}

	// Nest a loop to check all columns in all rows.
	for (int i = InPosition.X; i < InPosition.X + InSlotSize.X; i++) {
		for (int k = InPosition.Y; k < InPosition.Y + InSlotSize.Y; k++) {
			// If it's not in the free space we are tracking (an optimization, since free space can be up to infinity)
			if (!TrackedFreeSpace.Contains(FIntPoint(i, k))) {
				// If <= the Y bound at this point, then the space is used by something. Else it is just untracked free space.
				if (k <= GridTrackedYBound) {
					return false;
				}
			}
		}
	}
	return true;
}

void UJigsawGridWidget::AddSlotToGrid(UWidget* InWidget, FIntPoint InPosition, FIntPoint InSlotSize) {
	if (!IsValid(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InWidget invalid."));
		return;
	}
	if (InSlotSize.X < 1 || InSlotSize.Y < 1) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InSlotSize < 1."));
		return;
	}
	if (!IsValid(GridCanvasWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("GridCanvasWidget invalid."));
		return;
	}

	UCanvasPanelSlot* CanvasSlot = GridCanvasWidget->AddChildToCanvas(InWidget);
	if (!IsValid(CanvasSlot)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("CanvasSlot invalid."));
		return;
	}

	// Set widget size
	CanvasSlot->SetSize(FVector2D(InSlotSize.X * SlotPixelSize, InSlotSize.Y * SlotPixelSize));

	// Validate position to avoid overlap
	FIntPoint ValidatedPosition = InPosition;
	if (!CanPlaceSlotAtPosition(ValidatedPosition, InSlotSize)) {
		GetFreePositionForSlot(ValidatedPosition, InSlotSize);
	}

	// Set widget position
	CanvasSlot->SetPosition(FVector2D(ValidatedPosition.X * SlotPixelSize, ValidatedPosition.Y * SlotPixelSize));

	// Track newly used space
	TArray<FIntPoint> UsedSpace;
	for (int i = ValidatedPosition.X; i < ValidatedPosition.X + InSlotSize.X; i++) {
		for (int k = ValidatedPosition.Y; k < ValidatedPosition.Y + InSlotSize.Y; k++) {
			FIntPoint Position = FIntPoint(i, k);
			UsedSpace.Add(Position);
			TrackedFreeSpace.Remove(Position);
		}
	}

	// Map slots to slot widget
	GridSlots.Add(InWidget, FS_JigsawGridSlotInfo(ValidatedPosition, InSlotSize, UsedSpace));

	// Track free space. This simplifies calculations later on.
	if (GridTrackedYBound < (ValidatedPosition.Y + InSlotSize.Y)) {
		for (int i = 0; i < ColumnsPerRow; i++) {
			// Start with GridTrackedYBound + 1 since anything lower has been handled already.
			// End with new rows + extra row.
			for (int k = GridTrackedYBound + 1; k <= ValidatedPosition.Y + InSlotSize.Y; k++) {
				FIntPoint PositionX = FIntPoint(i, k);
				bool bIsTrackedUsedSpace = false;

				for (const TPair<UWidget*, FS_JigsawGridSlotInfo>& SlotX : GetSlots()) {
					if (SlotX.Value.UsedSpace.Contains(PositionX)) {
						bIsTrackedUsedSpace = true;
						break;
					}
				}
				if (!bIsTrackedUsedSpace) {
					TrackedFreeSpace.Add(PositionX);
				}
			}
		}
		GridTrackedYBound = ValidatedPosition.Y + InSlotSize.Y;
	}
}

void UJigsawGridWidget::RemoveSlotFromGrid(UWidget* InWidget) {
	if (!IsValid(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("InWidget invalid."));
		return;
	}
	const FS_JigsawGridSlotInfo* SlotInfoPtr = GridSlots.Find(InWidget);
	if (!SlotInfoPtr) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("SlotInfo not found for this widget."));
		return;
	}
	FS_JigsawGridSlotInfo SlotInfo = *SlotInfoPtr;

	// Remove the widget
	GridSlots.Remove(InWidget);
	// Free up space
	for (FIntPoint PositionX : SlotInfo.UsedSpace) {
		TrackedFreeSpace.Add(PositionX);
	}
}

bool UJigsawGridWidget::GetFreePositionForSlot(FIntPoint& OutPosition, FIntPoint InSlotSize) const {
	OutPosition = FIntPoint(0, 0);
	if (InSlotSize.X < 1 || InSlotSize.Y < 1) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InSlotSize < 1."));
		return false;
	}
	if (InSlotSize.X > ColumnsPerRow) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("The horizontal size of InSlotSize is larger than the amount of columns on this grid, so it does not fit."));
		return false;
	}

	if (GetSlots().Num() == 0) {
		// It must fit at OutPosition (set to 0,0 earlier this method) since nothing has been added yet.
		return true;
	}
	for (const FIntPoint& SpaceX : TrackedFreeSpace) {
		if (CanPlaceSlotAtPosition(SpaceX, InSlotSize)) {
			OutPosition = FIntPoint(SpaceX.X, SpaceX.Y);
			return true;
		}
	}
	return false;
}

TArray<FIntPoint> UJigsawGridWidget::GetOuterRadiusSpaceOfSpace(TArray<FIntPoint> InSpace, int32 InRadius) const {
	TArray<FIntPoint> OuterRadiusSpace;
	if (InRadius < 1) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("InRadius < 1."));
		return OuterRadiusSpace;
	}
	if (InSpace.Num() == 0) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("InSpace empty."));
		return OuterRadiusSpace;
	}

	// Set initial values
	int32 XMin = InSpace[0].X;
	int32 XMax = InSpace[0].X;
	int32 YMin = InSpace[0].Y;
	int32 YMax = InSpace[0].Y;

	// Calculate box boundaries
	for (const FIntPoint& SpaceX : InSpace) {
		if (SpaceX.X < XMin) {
			XMin = SpaceX.X;
		}
		if (SpaceX.X > XMax) {
			XMax = SpaceX.X;
		}
		if (SpaceX.Y < YMin) {
			YMin = SpaceX.Y;
		}
		if (SpaceX.Y > YMax) {
			YMax = SpaceX.Y;
		}
	}
	XMin -= InRadius;
	XMax += InRadius;
	YMin -= InRadius;
	YMax += InRadius;

	// Create radius space
	for (int k = YMin; k <= YMax; k++) {
		for (int i = XMin; i <= XMax; i++) {
			FIntPoint Position = FIntPoint(i, k);
			if (!InSpace.Contains(Position)) {
				OuterRadiusSpace.Add(Position);
			}
		}
	}

	return OuterRadiusSpace;
}

TArray<UWidget*> UJigsawGridWidget::FindWidgetsInSpace(TArray<FIntPoint> InSpace, bool bStopAtFirstEncounter) const {
	TArray<UWidget*> OutWidgets;
	for (const TPair<UWidget*, FS_JigsawGridSlotInfo>& SlotX : GetSlots()) {
		// For each slot, check if each used "grid space" of the slot is within InSpace.
		for (const FIntPoint& SpaceX : SlotX.Value.UsedSpace) {
			if (InSpace.Contains(SpaceX)) {
				OutWidgets.Add(SlotX.Key);
				break;
			}
		}
		if (bStopAtFirstEncounter && OutWidgets.Num() > 0) {
			break;
		}
	}
	return OutWidgets;
}