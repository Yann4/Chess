// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessBlock.h"
#include "PieceActor.h"
#include "Board.h"

#include "ChessBlockGrid.generated.h"

using namespace Chess;

/** Class used to spawn blocks and manage score */
UCLASS(minimalapi)
class AChessBlockGrid : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	TArray<AChessBlock*> m_Grid;
	Board m_Board;

	AChessBlock* SelectedSquare = nullptr;
public:
	AChessBlockGrid();

	/** Spacing of blocks */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	float BlockSpacing;

	UPROPERTY(Category = Grid, EditAnywhere)
	TSubclassOf<APieceActor> PieceTemplate;

protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

private:
	APieceActor* SpawnPiece(int32 Piece, AChessBlock& square);
	void MovePiece(APieceActor& Piece, AChessBlock& OriginSquare, AChessBlock* TargetSquare);

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }

	UFUNCTION()
	void OnSquareClicked(AChessBlock* square);

	UFUNCTION()
	void CancelMove();
};



