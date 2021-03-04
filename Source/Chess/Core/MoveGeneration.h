#pragma once

#include "CoreMinimal.h"
#include <vector>

#include "State.h"
#include "Move.h"

namespace Chess
{
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

}