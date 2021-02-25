// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <array>
#include <map>
#include <vector>

/**
 * 
 */
 
 //Used to disambiguate a constructor. Not nice really
class PiecePromotion
{
public:
	explicit PiecePromotion(int32 type) : PromoteTo(type) {}

	int32 PromoteTo;
};

inline bool operator==(const PiecePromotion& lhs, const PiecePromotion& rhs) { return lhs.PromoteTo == rhs.PromoteTo; }
inline bool operator!=(const PiecePromotion& lhs, const PiecePromotion& rhs) { return !operator==(lhs, rhs); }


namespace Piece
{
	const int32 None = 0;
	const int32 King = 1;
	const int32 Pawn = 2;
	const int32 Knight = 3;
	const int32 Bishop = 4;
	const int32 Rook = 5;
	const int32 Queen = 6;

	const int32 White = 8;
	const int32 Black = 16;

	const int32 ClassMask = 0x7;

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
	int32 StartSquare;
	int32 TargetSquare;

	int32 SecondaryStart;
	int32 SecondaryTarget;

	int32 Colour;

	int32 AllowsEnPassant;
	
	int8 PreventsCastling;
	int8 Castle;

	PiecePromotion Promote;

	ChessMove() : 
		StartSquare(-1), TargetSquare(-1),
		SecondaryStart(-1), SecondaryTarget(-1),
		Colour(-1),
		AllowsEnPassant(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Regular move/capture
	ChessMove(int32 Start, int32 Target, int32 PlayerColour) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(-1), SecondaryTarget(-1),
		Colour(PlayerColour),
		AllowsEnPassant(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Pawn move that allows en passent
	ChessMove(int32 Start, int32 Target, int32 PlayerColour, int32 EnPassant) :
		StartSquare(Start), TargetSquare(Target), 
		SecondaryStart(-1), SecondaryTarget(-1), 
		Colour(PlayerColour),
		AllowsEnPassant(EnPassant), 
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Pawn capturing en passent
	ChessMove(int32 Start, int32 Target, int32 PlayerColour, int32 secondaryStart, int32 secondaryTarget) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(secondaryStart), SecondaryTarget(secondaryTarget),
		Colour(PlayerColour),
		AllowsEnPassant(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(-1)
	{}

	//Pawn promotion
	explicit ChessMove(int32 Start, int32 Target, int32 PlayerColour, PiecePromotion promote) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(-1), SecondaryTarget(-1), 
		Colour(PlayerColour),
		AllowsEnPassant(-1),
		PreventsCastling(Castling::None), Castle(Castling::None),
		Promote(promote)
	{}

	//King move
	ChessMove(int32 Start, int32 Target, int32 PlayerColour, int8 StopsCastle) :
		StartSquare(Start), TargetSquare(Target),
		SecondaryStart(-1), SecondaryTarget(-1), 
		Colour(PlayerColour),
		AllowsEnPassant(-1),
		PreventsCastling(StopsCastle), Castle(Castling::None),
		Promote(-1)
	{}

	//Castling
	ChessMove(int32 PlayerColour, int8 castle) :
		StartSquare(0), TargetSquare(0), 
		Colour(PlayerColour),
		AllowsEnPassant(-1),
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
	ThreatMap(Board* board, int32 colour) : Colour(colour), Map(0) { CalculateMap(board); }
	void CalculateMap(Board* board);
	bool IsThreatened(int64 square) const { return (Map & (1ULL << square)); }
private:
	void SetThreatened(int64 square) { Map |= 1ULL << square; }
private:
	int32 Colour;
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
	std::vector<ChessMove> GenerateMoves(int32 colour, bool calculateThreat = false) const;

	bool IsColour(int32 piece, int32 colour) const { return (piece & colour) == colour; }
	bool IsType(int32 piece, int32 type) const { return (piece & Piece::ClassMask) == type; }
	bool IsSlidingPiece(int32 piece) const { return IsType(piece, Piece::Bishop) || IsType(piece, Piece::Rook) || IsType(piece, Piece::Queen); }
	
	int32 GetEnPassentTarget() const { return EnPassentTarget; }
	int32 GetColourToMove() const { return ColourToMove; }
	int8 GetCastleAvailability(int32 colour) const { return IsColour(colour, Piece::White) ? WhiteCastleAvailable : BlackCastleAvailable; }
	std::string PieceName(int32 piece) const;

private:
	std::array<int32, 64> ReadFEN(const std::string& fen);
	void PrecomputeMoveData();

	void GenerateSlidingMoves(int32 startSquare, int32 piece, std::vector<ChessMove>& moves) const;
	void GenerateKnightMoves(int32 startSquare, int32 piece, std::vector<ChessMove>& moves) const;
	void GeneratePawnMoves(int32 startSquare, int32 piece, std::vector<ChessMove>& moves) const;
	void GeneratePawnAttacks(int32 startSquare, int32 piece, std::vector<ChessMove>& moves, bool calculateThreat = false) const;
	void GenerateKingMoves(int32 startSquare, int32 piece, std::vector<ChessMove>& moves) const;

	bool IsPinned(int32 square) const;
	bool IsSquareThreatened(int32 square, int32 friendlyColour) const;

	int32 RankIndex(int32 square) const { return square >> 3; }
	int32 FileIndex(int32 square) const { return square & 0b000111; }
	int32 IndexFromCoord(int32 rank, int32 file) const { return rank * 8 + file; }

public:
	std::array<int32, 64> Squares;
	ThreatMap WhiteThreatening, BlackThreatening;

private:
	int32 ColourToMove;
	int8 WhiteCastleAvailable;
	int8 BlackCastleAvailable;

	const int32 NO_EN_PASSENT = -1;
	int32 EnPassentTarget = NO_EN_PASSENT;

	static std::map<char, int32> PieceTypeFromSymbol;
	static const std::string StandardStartFEN;

	static const int32 DirectionOffsets[];
	static int32 NumSquaresToEdge[64][8];
};
