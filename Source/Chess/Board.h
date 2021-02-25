// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <map>
#include <vector>

/**
 * 
 */
 
 //Used to disambiguate a constructor. Not nice really
class PiecePromotion
{
public:
	explicit PiecePromotion(int8 type) : PromoteTo(type) {}

	int8 PromoteTo;
};

inline bool operator==(const PiecePromotion& lhs, const PiecePromotion& rhs) { return lhs.PromoteTo == rhs.PromoteTo; }
inline bool operator!=(const PiecePromotion& lhs, const PiecePromotion& rhs) { return !operator==(lhs, rhs); }

namespace Piece
{
	const int8 None = 0;
	const int8 King = 1;
	const int8 Pawn = 2;
	const int8 Knight = 3;
	const int8 Bishop = 4;
	const int8 Rook = 5;
	const int8 Queen = 6;

	const int8 White = 8;
	const int8 Black = 16;

	const int8 ClassMask = 0x7;

	namespace Promotion
	{
		static const PiecePromotion None = PiecePromotion(-1);
		static const PiecePromotion Queen = PiecePromotion(Piece::Queen);
		static const PiecePromotion Rook = PiecePromotion(Piece::Rook);
		static const PiecePromotion Knight = PiecePromotion(Piece::Knight);
		static const PiecePromotion Bishop = PiecePromotion(Piece::Bishop);
		static const PiecePromotion AllPromotions[4]{ Queen, Rook, Knight, Bishop };
	}
}

namespace Castling
{
	const int8 None = 0;
	const int8 Queenside = 1;
	const int8 Kingside = 2;
};

struct CHESS_API ChessMove
{
public:
	int8 StartSquare;
	int8 TargetSquare;

	int8 SecondaryStart;
	int8 SecondaryTarget;

	int8 Colour;

	int8 EnPassentTarget;
	
	int8 PreventsCastling;
	int8 Castle;

	PiecePromotion Promote;

	ChessMove() : 
		StartSquare(-1), TargetSquare(-1),
		SecondaryStart(-1), SecondaryTarget(-1),
		Colour(-1),
		EnPassentTarget(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Regular move/capture
	ChessMove(int8 Start, int8 Target, int8 PlayerColour) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(-1), SecondaryTarget(-1),
		Colour(PlayerColour),
		EnPassentTarget(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Pawn move that allows en passent
	ChessMove(int8 Start, int8 Target, int8 PlayerColour, bool EnPassant) :
		StartSquare(Start), TargetSquare(Target), 
		SecondaryStart(-1), SecondaryTarget(-1), 
		Colour(PlayerColour),
		EnPassentTarget(Start + (PlayerColour == Piece::White ? 8 : -8)), 
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Pawn capturing en passent
	ChessMove(int8 Start, int8 Target, int8 PlayerColour, int8 secondaryStart, int8 secondaryTarget) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(secondaryStart), SecondaryTarget(secondaryTarget),
		Colour(PlayerColour),
		EnPassentTarget(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Pawn promotion
	ChessMove(int8 Start, int8 Target, int8 PlayerColour, PiecePromotion promote) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(-1), SecondaryTarget(-1), 
		Colour(PlayerColour),
		EnPassentTarget(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(promote)
	{}

	//King move
	ChessMove(int8 Start, int8 Target, int8 PlayerColour, int8 StopsCastle) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(-1), SecondaryTarget(-1), 
		Colour(PlayerColour),
		EnPassentTarget(-1),
		PreventsCastling(StopsCastle), Castle(Castling::None),
		Promote(-1)
	{}

	//Castling
	ChessMove(int8 PlayerColour, int8 castle) :
		StartSquare(0), TargetSquare(0), 
		Colour(PlayerColour),
		EnPassentTarget(-1),
		PreventsCastling(Castling::Kingside | Castling::Queenside), Castle(castle),
		Promote(-1)
	{
		StartSquare = PlayerColour == Piece::White ? 4 : 60;

		if (castle == Castling::Kingside)
		{
			TargetSquare = PlayerColour == Piece::White ? 6 : 62;
		}
		else if (castle == Castling::Queenside)
		{
			TargetSquare = PlayerColour == Piece::White ? 2 : 58;
		}

		//Handle rook, king has already moved
		if ((PlayerColour & Piece::White) == Piece::White)
		{
			SecondaryStart = Castle == Castling::Queenside ? 0 : 7;
			SecondaryTarget = Castle == Castling::Queenside ? 3 : 5;
		}
		else
		{
			SecondaryStart = Castle == Castling::Queenside ? 56 : 63;
			SecondaryTarget = Castle == Castling::Queenside ? 59 : 61;
		}
	}
};

class Board;
struct ThreatMap
{
public:
	ThreatMap() {}
	ThreatMap(Board* board, int8 colour) : Colour(colour), Map(0) { CalculateMap(board); }
	void CalculateMap(Board* board);
	bool IsThreatened(int8 square) const { return (Map & (1ULL << square)); }
private:
	void SetThreatened(int8 square) { Map |= 1ULL << square; }
private:
	int8 Colour;
	int64 Map;
};

inline bool operator==(const ChessMove& lhs, const ChessMove& rhs) { return lhs.StartSquare == rhs.StartSquare && lhs.TargetSquare == rhs.TargetSquare && lhs.Colour == rhs.Colour; }
inline bool operator!=(const ChessMove& lhs, const ChessMove& rhs) { return !operator==(lhs, rhs); }

class CHESS_API Board
{
public:
	Board();
	~Board();

	bool MakeMove(ChessMove& move);
	bool IsValidMove(ChessMove& move) const;
	std::vector<ChessMove> GenerateMoves(int8 colour, bool calculateThreat = false) const;

	bool IsColour(int8 piece, int8 colour) const { return (piece & colour) == colour; }
	bool IsType(int8 piece, int8 type) const { return (piece & Piece::ClassMask) == type; }
	bool IsSlidingPiece(int8 piece) const { return IsType(piece, Piece::Bishop) || IsType(piece, Piece::Rook) || IsType(piece, Piece::Queen); }
	
	int8 GetEnPassentTarget() const { return EnPassentTarget; }
	int8 GetColourToMove() const { return ColourToMove; }
	int8 GetCastleAvailability(int8 colour) const { return IsColour(colour, Piece::White) ? WhiteCastleAvailable : BlackCastleAvailable; }
	std::string PieceName(int8 piece) const;

private:
	void ReadFEN(const std::string& fen, int8* board);
	void PrecomputeMoveData();

	void GenerateSlidingMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const;
	void GenerateKnightMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const;
	void GeneratePawnMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const;
	void GeneratePawnAttacks(int8 startSquare, int8 piece, std::vector<ChessMove>& moves, bool calculateThreat = false) const;
	void GenerateKingMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const;

	bool IsPinned(int8 square) const;
	bool IsSquareThreatened(int8 square, int8 friendlyColour) const;

	int8 RankIndex(int8 square) const { return square >> 3; }
	int8 FileIndex(int8 square) const { return square & 0b000111; }
	int8 IndexFromCoord(int8 rank, int8 file) const { return rank * 8 + file; }

public:
	int8 Squares[64];
	ThreatMap WhiteThreatening, BlackThreatening;

private:
	int8 ColourToMove;
	int8 WhiteCastleAvailable;
	int8 BlackCastleAvailable;

	static const int8 NO_EN_PASSENT = -1;
	int8 EnPassentTarget = NO_EN_PASSENT;

	static std::map<char, int8> PieceTypeFromSymbol;
	static const std::string StandardStartFEN;

	static const int8 DirectionOffsets[];
	static int8 NumSquaresToEdge[64][8];
};
