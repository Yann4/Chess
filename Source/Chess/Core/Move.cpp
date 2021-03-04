#include "Move.h"
#include "Utils.h"
#include "State.h"

namespace Chess
{
	std::string Move::GenerateAlgebraicName(const State& board) const
	{
		if (Castle == Constants::Castling::Kingside)
		{
			return "0-0";
		}
		else if (Castle == Constants::Castling::Queenside)
		{
			return "0-0-0";
		}
		else if (Promote != Constants::Piece::None)
		{
			return Utils::SquareName(StartSquare) + "=" + Utils::AlgebraicName(Promote);
		}

		std::string move = Utils::AlgebraicName(board.Squares[StartSquare]) + Utils::SquareName(StartSquare); //TODO - Square name should only be used to disambiguate

		if (board.Squares[TargetSquare] != Constants::Piece::None)
		{
			move += "x";
		}

		move += Utils::SquareName(TargetSquare);

		if (board.IsKingThreatened(Utils::IsColour(Colour, Constants::Piece::White) ? Constants::Piece::Black : Constants::Piece::White))
		{
			//TODO: Determine checkmate
			move += "+";
		}

		return move;
	}
}