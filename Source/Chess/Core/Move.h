#pragma once

#include "CoreMinimal.h"
#include <string>

#include "Constants.h"

namespace Chess
{
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
			Promote(promotion), AlgebraicName(/*GenerateAlgebraicName(board)*/) //TODO: Re-add move naming in a way that isn't recursive
		{ }

		std::string GenerateAlgebraicName(const State& board) const;
		std::string AlgebraicName;
	};

	inline bool operator==(const Move& lhs, const Move& rhs) { return lhs.StartSquare == rhs.StartSquare && lhs.TargetSquare == rhs.TargetSquare && lhs.Colour == rhs.Colour; }
	inline bool operator!=(const Move& lhs, const Move& rhs) { return !operator==(lhs, rhs); }
}