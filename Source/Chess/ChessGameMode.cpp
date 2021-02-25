// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessGameMode.h"
#include "ChessPlayerController.h"
#include "ChessPawn.h"

AChessGameMode::AChessGameMode()
{
	// no pawn by default
	DefaultPawnClass = AChessPawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = AChessPlayerController::StaticClass();
}
