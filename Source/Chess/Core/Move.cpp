#include "Move.h"
#include "Utils.h"
#include "State.h"
#include "MoveGeneration.h"

#include <algorithm>

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

		/*
			We need to figure out if another piece of the same type can move to the same target square
			Because if it can, we need to disambiguate the move. 
			e.g if both rooks, one on g1 one on a2 can move to A1, we should have Rga1 or Raa1 so we know
			which one moved
		*/

		int8 pieceToMove = board.Squares[StartSquare];
		std::string move = Utils::AlgebraicName(pieceToMove);
		
		//We only need to do this if there are multiple pieces of the same type. So gather all pieces of that type on the board, remove the one that's making
		//the move & skip if there's only one
		std::vector<int8> piecesOfSameType = board.FindPiece(pieceToMove);
		piecesOfSameType.erase(std::remove_if(piecesOfSameType.begin(), piecesOfSameType.end(), [pieceToMove](int8 piece) { return piece == pieceToMove; }));
		if (piecesOfSameType.size() > 1)
		{
			//Now generate all of the valid moves for the other pieces
			//TODO: I'm sure this can be done more efficiently
			std::vector<Move> validMoves;
			if (Utils::IsType(pieceToMove, Constants::Piece::Rook) || Utils::IsType(pieceToMove, Constants::Piece::Bishop))
			{
				for (int8 piece : piecesOfSameType)
				{
					MoveGeneration::GenerateSlidingMoves(board, piece, validMoves);
				}
			}
			else if (Utils::IsType(pieceToMove, Constants::Piece::Knight))
			{
				for (int8 piece : piecesOfSameType)
				{
					MoveGeneration::GenerateKnightMoves(board, piece, validMoves);
				}
			}
			else if (Utils::IsType(pieceToMove, Constants::Piece::Pawn))
			{
				for (int8 piece : piecesOfSameType)
				{
					MoveGeneration::GeneratePawnMoves(board, piece, validMoves);
					MoveGeneration::GeneratePawnAttacks(board, piece, validMoves);
				}
			}

			//Loop over all of those moves, if any have the same target, voila, ambiguity to be resolved
			for (Move& alternateMove: validMoves)
			{
				if (alternateMove.TargetSquare == TargetSquare)
				{
					move += Utils::SquareName(StartSquare);
					break;
				}
			}
		}

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