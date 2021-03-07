#pragma once

#include "CoreMinimal.h"
#include <string>
#include <vector>

#include "Constants.h"
#include "Move.h"
#include "ThreatMap.h"

namespace Chess
{
	struct State
	{
	public:
		int8 Squares[64];
		int8 ColourToMove;
		int8 WhiteCastleAvailable;
		int8 BlackCastleAvailable;
		int8 EnPassentTarget;

		ThreatMap WhiteThreatMap;
		ThreatMap BlackThreatMap;

		State(const std::string& fen);
		State() :
			ColourToMove(Constants::Piece::White), WhiteCastleAvailable(Constants::Castling::Both), BlackCastleAvailable(Constants::Castling::Both),
			EnPassentTarget(Constants::NO_EN_PASSENT), WhiteThreatMap(*this, Constants::Piece::White), BlackThreatMap(*this, Constants::Piece::Black)
		{
			memset(Squares, 0, 64);
		}

		void Update(const Move& move);
		void UpdateThreatMaps();

		bool IsSquareThreatened(int8 square, int8 friendlyColour) const;
		bool IsKingThreatened(int8 colour) const;

		std::vector<int8> FindPiece(int8 piece) const;
	};
}