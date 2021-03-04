#pragma once

#include "CoreMinimal.h"
#include <string>

#include "Constants.h"

namespace Chess
{
	namespace Utils
	{
		inline int8 GetColour(int8 piece) { return piece & ~Constants::Piece::ClassMask; }
		inline bool IsColour(int8 piece, int8 colour) { return (piece & colour) == colour; }
		inline bool IsType(int8 piece, int8 type) { return (piece & Constants::Piece::ClassMask) == type; }
		inline bool IsSlidingPiece(int8 piece) { return IsType(piece, Constants::Piece::Bishop) || IsType(piece, Constants::Piece::Rook) || IsType(piece, Constants::Piece::Queen); }

		inline int8 RankIndex(int8 square) { return square >> 3; }
		inline int8 FileIndex(int8 square) { return square & 0b000111; }
		inline int8 IndexFromCoord(int8 rank, int8 file) { return rank * 8 + file; }

		std::string PieceName(int8 piece);
		std::string AlgebraicName(int8 piece);

		static std::string RankNames = "12345678";
		static std::string FileNames = "abcdefgh";

		inline std::string SquareName(int8 square)
		{
			std::string name;
			name.append(1, FileNames[FileIndex(square)]);
			name.append(1, RankNames[RankIndex(square)]);
			return name;
		}

		inline int8 SquareFromName(const char* name)
		{
			size_t file = FileNames.find(name[0]);
			size_t rank = RankNames.find(name[1]);
			return IndexFromCoord(rank, file);
		}
	}
}