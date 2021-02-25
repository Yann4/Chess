// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessBlock.h"
#include "ChessBlockGrid.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"

AChessBlock::AChessBlock()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> LightMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> DarkMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> HighlightMaterial;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			, BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
			, LightMaterial(TEXT("/Game/Puzzle/Meshes/WhiteMaterial.WhiteMaterial"))
			, DarkMaterial(TEXT("/Game/Puzzle/Meshes/BlackMaterial.BlackMaterial"))
			, HighlightMaterial(TEXT("/Game/Puzzle/Meshes/HighlightMaterial.HighlightMaterial"))
		{
		}
	};

	static FConstructorStatics ConstructorStatics;

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Create static mesh component
	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh0"));
	BlockMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	BlockMesh->SetRelativeScale3D(FVector(1.f,1.f,0.25f));
	BlockMesh->SetRelativeLocation(FVector(0.f,0.f,25.f));
	BlockMesh->SetMaterial(0, ConstructorStatics.LightMaterial.Get());
	BlockMesh->SetupAttachment(DummyRoot);
	BlockMesh->OnClicked.AddDynamic(this, &AChessBlock::BlockClicked);
	BlockMesh->OnInputTouchBegin.AddDynamic(this, &AChessBlock::OnFingerPressedBlock);

	// Save a pointer to the materials
	BaseMaterial = ConstructorStatics.BaseMaterial.Get();
	LightMaterial = ConstructorStatics.LightMaterial.Get();
	DarkMaterial = ConstructorStatics.DarkMaterial.Get();
	HighlightMaterial = ConstructorStatics.HighlightMaterial.Get();
}

void AChessBlock::BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
	HandleClicked();
}


void AChessBlock::OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	HandleClicked();
}

void AChessBlock::HandleClicked()
{
	OwningGrid->OnSquareClicked(this);
}

void AChessBlock::SetLightSquare(bool isLight)
{
	bIsLightSquare = isLight;
	BlockMesh->SetMaterial(0, isLight ? LightMaterial : DarkMaterial);
}

void AChessBlock::OccupySquare(APieceActor* piece)
{
	OccupyingPiece = piece;
}

void AChessBlock::Highlight(bool bOn)
{
	BlockMesh->SetMaterial(0, bOn ? HighlightMaterial :
		(bIsLightSquare ? LightMaterial : DarkMaterial));
}
