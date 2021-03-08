// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <stack>

#include "Constants.h"
#include "Move.h"
#include "Utils.h"
#include "State.h"

/**
 * 
 */
namespace Chess
{
	class Board
	{
	public:
		Board();

		bool MakeMove(Move& move);
		bool IsValidMove(Move& move) const;
		bool UnmakeMove();

		inline int8 GetEnPassentTarget() const { return BoardState.EnPassentTarget; }
		inline int8 GetColourToMove() const { return BoardState.ColourToMove; }
		inline int8 GetCastleAvailability(int8 colour) const { return Utils::IsColour(colour, Constants::Piece::White) ? BoardState.WhiteCastleAvailable : BoardState.BlackCastleAvailable; }

		static bool DoesMoveExposeKing(const State& state, const Move& move, int8 king);

	public:
		State BoardState;
		std::stack<State> StateHistory;
	};
}