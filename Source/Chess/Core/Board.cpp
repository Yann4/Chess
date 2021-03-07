// Fill out your copyright notice in the Description page of Project Settings.

#include "Board.h"
#include <assert.h>

#include "MoveGeneration.h"

namespace Chess
{
	using namespace Constants;

	Board::Board():
		BoardState(StandardStartFEN)
	{
		PrecomputeMoveData(); //TODO: Make this happen at compile time
	}

	bool Board::MakeMove(Move& move)
	{
		if (IsValidMove(move))
		{
			BoardState.Update(move);
			return true;
		}

		return false;
	}

	bool Board::IsValidMove(Move& move) const
	{
		//TODO: This is really slow. Fix

		std::vector<Move> validMoves = MoveGeneration::GenerateMoves(BoardState, BoardState.ColourToMove);

		int8 king = Constants::DEFAULT;
		for (int8 square = 0; square < 64; square++)
		{
			if (Utils::IsType(BoardState.Squares[square], Piece::King) && Utils::IsColour(BoardState.Squares[square], BoardState.ColourToMove))
			{
				king = square;
				break;
			}
		}

		assert(king != Constants::DEFAULT);
		for (int idx = validMoves.size() - 1; idx >= 0; idx--)
		{
			if (Board::DoesMoveExposeKing(BoardState, validMoves[idx], king))
			{
				validMoves.erase(validMoves.begin() + idx);
			}
		}

		auto validatedMove = std::find(validMoves.begin(), validMoves.end(), move);
		bool moveIsValid = validatedMove != validMoves.end();
		if (moveIsValid)
		{
			move = *validatedMove;
		}

		return moveIsValid;
	}

	bool Board::DoesMoveExposeKing(const State& state, const Move& move, int8 king)
	{
		State afterMove(state);
		int8 colour = state.ColourToMove;

		afterMove.Update(move);
		afterMove.UpdateThreatMaps();

		int8 kingSquare = Utils::IsType(state.Squares[move.StartSquare], Piece::King) ? move.TargetSquare : king;
		return afterMove.IsSquareThreatened(kingSquare, colour);
	}
}