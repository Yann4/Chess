#include "ThreatMap.h"

#include "Move.h"
#include "Constants.h"
#include "MoveGeneration.h"

namespace Chess
{
	void ThreatMap::CalculateMap(const State& board)
	{
		Map = 0;
		for (const Move& move : MoveGeneration::GenerateMoves(board, Colour, true))
		{
			SetThreatened(move.TargetSquare);
		}
	}
}