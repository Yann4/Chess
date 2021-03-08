// Fill out your copyright notice in the Description page of Project Settings.

#include "Board.h"
#include <assert.h>

#include "MoveGeneration.h"

namespace Chess
{
	using namespace Constants;

	Board::Board():
		BoardState(StandardStartFEN)
	{}

	bool Board::MakeMove(Move& move)
	{
		if (IsValidMove(move))
		{
			StateHistory.push(BoardState);

			BoardState.Update(move);
			return true;
		}

		return false;
	}

	bool Board::UnmakeMove()
	{
		if (StateHistory.size() > 0)
		{
			BoardState = StateHistory.top();
			StateHistory.pop();
			return true;
		}

		return false;
	}

	bool Board::IsValidMove(Move& move) const
	{
		std::vector<Move> validMoves = MoveGeneration::GenerateMoves(BoardState, BoardState.ColourToMove);

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