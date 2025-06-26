/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "WorldCursorModifierComponent.h"


// Cursor

bool UWorldCursorModifierComponent::GetDesiresCenteredWorldCursor() const {
	return bDesiresCenteredWorldCursor;
}

void UWorldCursorModifierComponent::SetDesiresCenteredWorldCursor(bool bInDesiresCenteredWorldCursor) {
	bDesiresCenteredWorldCursor = bInDesiresCenteredWorldCursor;
	OnDesiresCenteredWorldCursorChanged.Broadcast(GetDesiresCenteredWorldCursor());
}