// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessBlockGrid.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

using namespace Chess::Constants;

AChessBlockGrid::AChessBlockGrid()
{
	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Set defaults
	BlockSpacing = 250.0f;

	m_Grid.Init(nullptr, 64);
}

void AChessBlockGrid::BeginPlay()
{
	Super::BeginPlay();

	int32 blockIdx = 0;
	// Number of blocks
	for (int32 File = 0; File < 8; File++)
	{
		for (int32 Rank = 0; Rank < 8; Rank++)
		{
			const float XOffset = File * BlockSpacing;
			const float YOffset = Rank * BlockSpacing;

			// Make position vector, offset from Grid location
			const FVector BlockLocation = FVector(XOffset, YOffset, 0.f) + GetActorLocation();

			// Spawn a block
			AChessBlock* NewBlock = GetWorld()->SpawnActor<AChessBlock>(BlockLocation, FRotator(0, 0, 0));
			NewBlock->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

			NewBlock->SetLightSquare((File + Rank) % 2 == 0);

			// Tell the block about its owner
			if (NewBlock != nullptr)
			{
				NewBlock->OwningGrid = this;
			}

			m_Grid[blockIdx++] = NewBlock;
		}
	}

	for (int32 idx = 0; idx < 64; idx++)
	{
		SpawnPiece(m_Board.BoardState.Squares[idx], *m_Grid[idx]);
	}

	for (int idx = 0; idx < 64; idx++)
	{
		m_Grid[idx]->Highlight(m_Board.BoardState.WhiteThreatMap.IsThreatened(idx));
	}
}

APieceActor* AChessBlockGrid::SpawnPiece(int32 Piece, AChessBlock& square)
{
	if (Piece == Piece::None)
	{
		return nullptr;
	}

	const FRotator Rotator = FRotator(0, 0, 0);
	const FVector Location = FVector::ZeroVector;

	APieceActor* spawnedPiece = Cast<APieceActor>(GetWorld()->SpawnActor(PieceTemplate, &Location, &Rotator));
	spawnedPiece->SetPieceType((Class)(Piece & Piece::ClassMask), (Piece & Piece::Black) == Piece::Black);
	
	spawnedPiece->MoveTo(square.GetActorLocation());
	square.OccupySquare(spawnedPiece);

	return spawnedPiece;
}

void AChessBlockGrid::MovePiece(APieceActor& Piece, AChessBlock& OriginSquare, AChessBlock* TargetSquare)
{
	int32 startIDX = m_Grid.Find(&OriginSquare);
	int32 endIDX = m_Grid.Find(TargetSquare);
	
	int32 p = m_Board.BoardState.Squares[startIDX];

	FString name(Utils::PieceName(p).c_str());
	FString start = FString(Utils::SquareName(startIDX).c_str());
	FString end = FString(Utils::SquareName(endIDX).c_str());
	UE_LOG(LogTemp, Display, TEXT("Moving %s from square %s (%d) to %s (%d)"), *name, *start, Utils::SquareFromName(Utils::SquareName(startIDX).c_str()), *end, Utils::SquareFromName(Utils::SquareName(endIDX).c_str()));

	Chess::Move move = Move::CreateMove(m_Board.BoardState, startIDX, endIDX, p & ~Piece::ClassMask);
	if (Utils::IsType(p, Piece::King))
	{
		if ((Utils::IsColour(p, Piece::White) && endIDX == 1 || endIDX == 2) || (Utils::IsColour(p, Piece::Black) && (endIDX == 57 || endIDX == 58)))
		{
			Chess::Move castle = Move::CreateCastlingMove(m_Board.BoardState, p & ~Piece::ClassMask, Castling::Queenside);
			if (m_Board.IsValidMove(castle))
			{
				move = castle;
				TargetSquare = m_Grid[move.TargetSquare];
			}
		}
		else if ((Utils::IsColour(p, Piece::White) && endIDX == 6) || (Utils::IsColour(p, Piece::Black) && endIDX == 62))
		{
			Chess::Move castle = Move::CreateCastlingMove(m_Board.BoardState, p & ~Piece::ClassMask, Castling::Kingside);
			if (m_Board.IsValidMove(castle))
			{
				move = castle;
				TargetSquare = m_Grid[move.TargetSquare];
			}
		}
	}

	if (m_Board.MakeMove(move))
	{
		UE_LOG(LogTemp, Display, TEXT("%s"), *FString(move.ToString().c_str()));
		OriginSquare.OccupyingPiece = nullptr;

		if (TargetSquare->OccupyingPiece != nullptr)
		{
			TargetSquare->OccupyingPiece->TakePiece();
			TargetSquare->OccupyingPiece = nullptr;
		}

		Piece.MoveTo(TargetSquare->GetActorLocation());
		TargetSquare->OccupySquare(&Piece);

		if (move.SecondaryStart != -1)
		{
			APieceActor* secondaryPiece = m_Grid[move.SecondaryStart]->OccupyingPiece;
			if (move.SecondaryTarget != -1) //Target is -1 in case of en passent, a real value in the case of castling
			{
				secondaryPiece->MoveTo(m_Grid[move.SecondaryTarget]->GetActorLocation());
				m_Grid[move.SecondaryTarget]->OccupySquare(secondaryPiece);
				m_Grid[move.SecondaryStart]->OccupyingPiece = nullptr;
			}
			else
			{
				secondaryPiece->TakePiece();
				m_Grid[move.SecondaryStart]->OccupyingPiece = nullptr;
			}
		}

		//At the moment, we just auto-promote to queen because I can't be arsed with the UI to select the underpromotions
		//But all that needs to happen is that the move needs to include the desired promotion & it should 'just work'
		if (move.Promote != Piece::None)
		{
			Piece.SetPieceType(static_cast<Class>(move.Promote), Utils::IsColour(p, Piece::Black));
		}

		for (int idx = 0; idx < 64; idx++)
		{
			m_Grid[idx]->Highlight(m_Board.BoardState.WhiteThreatMap.IsThreatened(idx));
		}
	}
}

void AChessBlockGrid::OnSquareClicked(AChessBlock* square)
{
	ensure(square != nullptr);

	//If this is the first click, we're picking a piece to move
	if (SelectedSquare == nullptr)
	{
		if (square->OccupyingPiece != nullptr)
		{
			SelectedSquare = square;
			//square->Highlight(true);
		}
	}
	else
	{
		MovePiece(*SelectedSquare->OccupyingPiece, *SelectedSquare, square);

		//SelectedSquare->Highlight(false);
		SelectedSquare = nullptr;
	}
}

void AChessBlockGrid::CancelMove()
{
	if (SelectedSquare != nullptr)
	{
		SelectedSquare->Highlight(false);
		SelectedSquare = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
