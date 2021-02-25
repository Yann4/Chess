// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessPlayerController.h"

AChessPlayerController::AChessPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}
