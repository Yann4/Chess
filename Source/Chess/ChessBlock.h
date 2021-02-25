// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceActor.h"
#include "ChessBlock.generated.h"

/** A block that can be clicked */
UCLASS(minimalapi)
class AChessBlock : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	/** StaticMesh component for the clickable block */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh;
public:
	AChessBlock();

	/** Are we currently active? */
	bool bIsLightSquare;

	/** Pointer to white material used on the focused block */
	UPROPERTY()
	class UMaterial* BaseMaterial;

	/** Pointer to blue material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* HighlightMaterial;
	
	/** Pointer to blue material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* LightMaterial;

	/** Pointer to orange material used on active blocks */
	UPROPERTY()
	class UMaterialInstance* DarkMaterial;

	/** Grid that owns us */
	UPROPERTY()
	class AChessBlockGrid* OwningGrid;

	UPROPERTY()
	APieceActor* OccupyingPiece = nullptr;

	/** Handle the block being clicked */
	UFUNCTION()
	void BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);

	/** Handle the block being touched  */
	UFUNCTION()
	void OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void SetLightSquare(bool isLight);

	UFUNCTION()
	void OccupySquare(APieceActor* piece);

	void HandleClicked();

	void Highlight(bool bOn);

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns BlockMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBlockMesh() const { return BlockMesh; }
};



