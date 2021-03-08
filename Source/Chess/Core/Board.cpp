// Fill out your copyright notice in the Description page of Project Settings.

#include "Board.h"

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
}