// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <map>
#include <vector>

/**
 * 
 */
 
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

	static int8 AllPromotions[4]{ Queen, Rook, Knight, Bishop };
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

	int8 Promote;

	//Regular move/capture
	static ChessMove CreateMove(int8 Start, int8 Target, int8 PlayerColour, int8 PreventsCastling = Castling::None)
	{
		return ChessMove(Start, Target, DEFAULT, DEFAULT, PlayerColour, DEFAULT, PreventsCastling);
	}

	//Pawn move that allows en passent
	static ChessMove CreateEnPassentMove(int8 Start, int8 Target, int8 PlayerColour)
	{
		return ChessMove(Start, Target, DEFAULT, DEFAULT, PlayerColour, Start + (PlayerColour == Piece::White ? 8 : -8));
	}

	//Pawn capturing en passent
	static ChessMove CreateEnPassentCapture(int8 Start, int8 Target, int8 PlayerColour, int8 passentPawn, int8 passentTarget)
	{
		return ChessMove(Start, Target, passentPawn, DEFAULT, PlayerColour, passentTarget);
	}

	//Pawn promotion
	static ChessMove CreatePromotionMove(int8 Start, int8 Target, int8 PlayerColour, int8 promote)
	{
		return ChessMove(Start, Target, DEFAULT, DEFAULT, PlayerColour, DEFAULT, DEFAULT, DEFAULT, promote);
	}

	//Castling
	static ChessMove CreateCastlingMove(int8 PlayerColour, int8 castle)
	{
		int8 Start = PlayerColour == Piece::White ? 4 : 60;
		int8 Target = -1;
		if (castle == Castling::Kingside)
		{
			Target = PlayerColour == Piece::White ? 6 : 62;
		}
		else if (castle == Castling::Queenside)
		{
			Target = PlayerColour == Piece::White ? 2 : 58;
		}

		int8 secondaryStart, secondaryTarget;
		//Handle rook, king has already moved
		if ((PlayerColour & Piece::White) == Piece::White)
		{
			secondaryStart = castle == Castling::Queenside ? 0 : 7;
			secondaryTarget = castle == Castling::Queenside ? 3 : 5;
		}
		else
		{
			secondaryStart = castle == Castling::Queenside ? 56 : 63;
			secondaryTarget = castle == Castling::Queenside ? 59 : 61;
		}

		return ChessMove(Start, Target, secondaryStart, secondaryTarget, PlayerColour, DEFAULT, Castling::Kingside | Castling::Queenside, castle);
	}

private:
	ChessMove(int8 start = -1, int8 target = -1, int8 secondaryStart = -1, int8 secondaryTarget = -1, int8 colour = -1, int8 enPassent = -1, int8 preventCastle = Castling::None, int8 castle = -1, int8 promotion = Piece::None) :
		StartSquare(start), TargetSquare(target),
		SecondaryStart(secondaryStart), SecondaryTarget(secondaryTarget),
		Colour(colour),
		EnPassentTarget(enPassent),
		PreventsCastling(preventCastle), Castle(castle),
		Promote(promotion)
	{}

	static const int8 DEFAULT = -1;
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
