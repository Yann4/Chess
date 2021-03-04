#pragma once
#include "CoreMinimal.h"

namespace Chess
{
	struct State;
	struct ThreatMap
	{
	public:
		ThreatMap(const State& board, int8 colour) : 
			Colour(colour), Map(0) 
		{ 
			CalculateMap(board); 
		}

		void CalculateMap(const State& board);
		inline bool IsThreatened(int8 square) const { return (Map & (1ULL << square)); }

	private:
		inline void SetThreatened(int8 square) { Map |= 1ULL << square; }

	private:
		const int8 Colour;
		int64 Map;
	};
}