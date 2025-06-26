/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "JigsawGridWidget.generated.h"

class USizeBox;
class UCanvasPanel;


USTRUCT(BlueprintType)
struct UIADDITIONSPLUGIN_API FS_JigsawGridSlotInfo {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		FIntPoint SlotPosition;

	UPROPERTY(BlueprintReadOnly)
		FIntPoint SlotSize;

	UPROPERTY(BlueprintReadOnly)
		TArray<FIntPoint> UsedSpace;

	// Initialize
	FS_JigsawGridSlotInfo()
		: SlotPosition (FIntPoint(0, 0))
		, SlotSize (FIntPoint(1, 1))
		, UsedSpace (TArray<FIntPoint>())
	{}
	FS_JigsawGridSlotInfo(FIntPoint InSlotPosition, FIntPoint InSlotSize, TArray<FIntPoint> InUsedSpace) 
		: SlotPosition (InSlotPosition)
		, SlotSize (InSlotSize)
		, UsedSpace (InUsedSpace)
	{}

	// Operators
	bool operator==(const FS_JigsawGridSlotInfo& Other) const {
		return (
			SlotPosition == Other.SlotPosition
			&& SlotSize == Other.SlotSize
			&& UsedSpace == Other.UsedSpace
		);
	}
};


/*
* A grid widget used to display widgets of custom sizes and shapes over X grid "slots".
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UJigsawGridWidget : public UUserWidget {
    GENERATED_BODY()

private:

	// Grid

	UPROPERTY()
		TMap<UWidget*, FS_JigsawGridSlotInfo> GridSlots = TMap<UWidget*, FS_JigsawGridSlotInfo>();

	UPROPERTY()
		int32 GridTrackedYBound = 0;

	UPROPERTY()
		TArray<FIntPoint> TrackedFreeSpace = TArray<FIntPoint>();

protected:

	// Setup

	/* The amount of columns in a row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup", meta = (ClampMin = "1", UIMin = "1"))
		int32 ColumnsPerRow = 10;

	/* The amount of columns in a row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup", meta = (ClampMin = "1", UIMin = "1"))
		int32 SlotPixelSize = 50;

	// Setup | Dependencies

	/* Dependency required from a deriving class. */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
		USizeBox* GridSizeBoxWidget = nullptr;
	
	/* Dependency required from a deriving class. */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
		UCanvasPanel* GridCanvasWidget = nullptr;

public:

private:

protected:

	// Setup

	virtual void NativePreConstruct() override;

	// Grid

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Grid")
		TArray<FIntPoint> GetTrackedFreeSpace() const;

public:

	// Grid
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Grid")
		TMap<UWidget*, FS_JigsawGridSlotInfo> GetSlots() const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
		void ClearGrid();

	/* Returns true if valid. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Grid")
		bool GetSlotInfoBySlotWidget(UWidget* InWidget, FS_JigsawGridSlotInfo& OutSlotInfo) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Grid")
		bool CanPlaceSlotAtPosition(FIntPoint InPosition, FIntPoint InSlotSize) const;

	/* Attempts to add the slot to the requested position, or adds it at a position where it fits. Overlap will not occur. */
	UFUNCTION(BlueprintCallable, Category = "Grid")
		void AddSlotToGrid(UWidget* InWidget, FIntPoint InPosition, FIntPoint InSlotSize);

	UFUNCTION(BlueprintCallable, Category = "Grid")
		void RemoveSlotFromGrid(UWidget* InWidget);

	/* Returns true if valid. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Grid")
		bool GetFreePositionForSlot(FIntPoint& OutPosition, FIntPoint InSlotSize) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Grid")
		TArray<FIntPoint> GetOuterRadiusSpaceOfSpace(TArray<FIntPoint> InSpace, int32 InRadius = 1) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Grid")
		TArray<UWidget*> FindWidgetsInSpace(TArray<FIntPoint> InSpace, bool bStopAtFirstEncounter = false) const;

};