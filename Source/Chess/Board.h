// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <map>
#include <vector>
#include <array>
#include <assert.h>

/**
 * 
 */
namespace Chess
{
	namespace Constants
	{
		const int8 NO_EN_PASSENT = -1;
		const int8 DEFAULT = -1;

		const std::string StandardStartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		namespace Castling
		{
			const int8 None = 0;
			const int8 Queenside = 1;
			const int8 Kingside = 2;
			const int8 Both = Queenside | Kingside;
		};

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
		}
		
		const int8 Promotions[4]{ Piece::Queen, Piece::Rook, Piece::Knight, Piece::Bishop };

		namespace
		{
			struct Initialiser
			{
				static std::map<char, int8> ConstructMap()
				{
					std::map<char, int8> map;
					
					map['k'] = Piece::King;
					map['p'] = Piece::Pawn;
					map['n'] = Piece::Knight;
					map['b'] = Piece::Bishop;
					map['r'] = Piece::Rook;
					map['q'] = Piece::Queen;

					return map;
				}
			};
		} //Anonymous namespace to initialise the PieceTypeFromSymbol map

		const std::map<char, int8> PieceTypeFromSymbol = Initialiser::ConstructMap();
		
		static int8 NumSquaresToEdge[64][8];
		static void PrecomputeMoveData()
		{
			for (int8 file = 0; file < 8; file++)
			{
				for (int8 rank = 0; rank < 8; rank++)
				{
					int8 numNorth = 7 - rank;
					int8 numSouth = rank;
					int8 numWest = file;
					int8 numEast = 7 - file;

					int8 squareIdx = rank * 8 + file;

					NumSquaresToEdge[squareIdx][0] = numNorth;
					NumSquaresToEdge[squareIdx][1] = numSouth;
					NumSquaresToEdge[squareIdx][2] = numWest;
					NumSquaresToEdge[squareIdx][3] = numEast;
					NumSquaresToEdge[squareIdx][4] = std::min(numNorth, numWest);
					NumSquaresToEdge[squareIdx][5] = std::min(numSouth, numEast);
					NumSquaresToEdge[squareIdx][6] = std::min(numNorth, numEast);
					NumSquaresToEdge[squareIdx][7] = std::min(numSouth, numWest);
				}
			}
		}
	}
	struct State;
	struct Move
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
		static Move CreateMove(const State& board, int8 Start, int8 Target, int8 PlayerColour, int8 PreventsCastling = Constants::Castling::None)
		{
			return Move(board, Start, Target, Constants::DEFAULT, Constants::DEFAULT, PlayerColour, Constants::DEFAULT, PreventsCastling);
		}

		//Pawn move that allows en passent
		static Move CreateEnPassentMove(const State& board, int8 Start, int8 Target, int8 PlayerColour)
		{
			return Move(board, Start, Target, Constants::DEFAULT, Constants::DEFAULT, PlayerColour, Start + (PlayerColour == Constants::Piece::White ? 8 : -8));
		}

		//Pawn capturing en passent
		static Move CreateEnPassentCapture(const State& board, int8 Start, int8 Target, int8 PlayerColour, int8 passentPawn, int8 passentTarget)
		{
			return Move(board, Start, Target, passentPawn, Constants::DEFAULT, PlayerColour, passentTarget);
		}

		//Pawn promotion
		static Move CreatePromotionMove(const State& board, int8 Start, int8 Target, int8 PlayerColour, int8 promote)
		{
			return Move(board, Start, Target, Constants::DEFAULT, Constants::DEFAULT, PlayerColour, Constants::DEFAULT, Constants::Castling::None, Constants::Castling::None, promote);
		}

		//Castling
		static Move CreateCastlingMove(const State& board, int8 PlayerColour, int8 castle)
		{
			using namespace Constants;

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

			return Move(board, Start, Target, secondaryStart, secondaryTarget, PlayerColour, Constants::DEFAULT, Castling::Both, castle);
		}

		std::string ToString() const { return AlgebraicName; }
	private:
		Move(const State& board, int8 start = Constants::DEFAULT, int8 target = Constants::DEFAULT, int8 secondaryStart = Constants::DEFAULT, int8 secondaryTarget = Constants::DEFAULT,
			int8 colour = Constants::DEFAULT, int8 enPassent = Constants::NO_EN_PASSENT, int8 preventCastle = Constants::Castling::None, int8 castle = Constants::Castling::None, int8 promotion = Constants::Piece::None) :
			StartSquare(start), TargetSquare(target),
			SecondaryStart(secondaryStart), SecondaryTarget(secondaryTarget),
			Colour(colour),
			EnPassentTarget(enPassent),
			PreventsCastling(preventCastle), Castle(castle),
			Promote(promotion), AlgebraicName(GenerateAlgebraicName(board))
		{ }

		std::string GenerateAlgebraicName(const State& board) const;
		std::string AlgebraicName;
	};

	inline bool operator==(const Move& lhs, const Move& rhs) { return lhs.StartSquare == rhs.StartSquare && lhs.TargetSquare == rhs.TargetSquare && lhs.Colour == rhs.Colour; }
	inline bool operator!=(const Move& lhs, const Move& rhs) { return !operator==(lhs, rhs); }

	namespace Utils
	{
		inline int8 GetColour(int8 piece) { return piece & ~Constants::Piece::ClassMask; }
		inline bool IsColour(int8 piece, int8 colour) { return (piece & colour) == colour; }
		inline bool IsType(int8 piece, int8 type) { return (piece & Constants::Piece::ClassMask) == type; }
		inline bool IsSlidingPiece(int8 piece) { return IsType(piece, Constants::Piece::Bishop) || IsType(piece, Constants::Piece::Rook) || IsType(piece, Constants::Piece::Queen); }

		inline int8 RankIndex(int8 square) { return square >> 3; }
		inline int8 FileIndex(int8 square) { return square & 0b000111; }
		inline int8 IndexFromCoord(int8 rank, int8 file) { return rank * 8 + file; }

		std::string PieceName(int8 piece);
		std::string AlgebraicName(int8 piece);

		static std::string RankNames = "12345678";
		static std::string FileNames = "abcdefgh";

		inline std::string SquareName(int8 square)
		{
			std::string name; 
			name.append(1, FileNames[FileIndex(square)]);
			name.append(1, RankNames[RankIndex(square)]); 
			return name;
		}

		inline int8 SquareFromName(const char* name)
		{
			size_t file = FileNames.find(name[0]);
			size_t rank = RankNames.find(name[1]);
			return IndexFromCoord(rank, file);
		}
	}

	struct State;
	struct ThreatMap
	{
	public:
		ThreatMap(const State& board, int8 colour) : Colour(colour), Map(0) { CalculateMap(board); }
		void CalculateMap(const State& board);
		bool IsThreatened(int8 square) const { return (Map & (1ULL << square)); }
	private:
		void SetThreatened(int8 square) { Map |= 1ULL << square; }

	private:
		const int8 Colour;
		int64 Map;
	};

	struct State
	{
	public:
		int8 Squares[64];
		int8 ColourToMove;
		int8 WhiteCastleAvailable;
		int8 BlackCastleAvailable;
		int8 EnPassentTarget;

		ThreatMap WhiteThreatMap;
		ThreatMap BlackThreatMap;

		State(const std::string& fen);
		State() :
			ColourToMove(Constants::Piece::White), WhiteCastleAvailable(Constants::Castling::Both), BlackCastleAvailable(Constants::Castling::Both),
			EnPassentTarget(Constants::NO_EN_PASSENT), WhiteThreatMap(*this, Constants::Piece::White), BlackThreatMap(*this, Constants::Piece::Black)
		{
			memset(Squares, 0, 64);
		}

		void UpdateBoard(const Move& move);

		void UpdateThreatMaps()
		{
			WhiteThreatMap.CalculateMap(*this);
			BlackThreatMap.CalculateMap(*this);
		}

		bool IsSquareThreatened(int8 square, int8 friendlyColour) const
		{
			const ThreatMap& threats = Utils::IsColour(friendlyColour, Constants::Piece::White) ? BlackThreatMap : WhiteThreatMap;
			return threats.IsThreatened(square);
		}

		bool IsKingThreatened(int8 colour) const
		{
			for (int idx = 0; idx < 64; idx++)
			{
				if (Utils::IsType(Squares[idx], Constants::Piece::King) && Utils::IsColour(Squares[idx], colour))
				{
					return IsSquareThreatened(idx, colour);
				}
			}

			assert(false);
			return false;
		}
	};

	namespace MoveGeneration
	{
		const int8 DirectionOffsets[8] = { 8, -8, -1, 1, 7, -7, 9, -9 };

		std::vector<Move> GenerateMoves(const State& board, int8 colour, bool calculateThreat = false);

		void GenerateSlidingMoves(const State& state, int8 startSquare, std::vector<Move>& moves);
		void GenerateKnightMoves(const State& state, int8 startSquare, std::vector<Move>& moves);
		void GeneratePawnMoves(const State& state, int8 startSquare, std::vector<Move>& moves);
		void GeneratePawnAttacks(const State& state, int8 startSquare, std::vector<Move>& moves, bool calculateThreat = false);
		void GenerateKingMoves(const State& state, int8 startSquare, std::vector<Move>& moves);
	}

	class Board
	{
	public:
		Board();

		bool MakeMove(Move& move);
		bool IsValidMove(Move& move) const;

		int8 GetEnPassentTarget() const { return BoardState.EnPassentTarget; }
		int8 GetColourToMove() const { return BoardState.ColourToMove; }
		int8 GetCastleAvailability(int8 colour) const { return Utils::IsColour(colour, Constants::Piece::White) ? BoardState.WhiteCastleAvailable : BoardState.BlackCastleAvailable; }

		static bool DoesMoveExposeKing(const State& state, const Move& move, int8 king);

	public:
		State BoardState;
	};
}