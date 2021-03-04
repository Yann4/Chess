// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <map>

namespace Chess
{
	namespace Constants
	{
		const int8 NO_EN_PASSENT = -1;
		const int8 DEFAULT = -1;

		const std::string StandardStartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		namespace Castling
		{
			const int8 None = 0;
			const int8 Queenside = 1;
			const int8 Kingside = 2;
			const int8 Both = Queenside | Kingside;
		};

		namespace Piece
		{
			const int8 None = 0;
			const int8 King = 1;
			const int8 Pawn = 2;
			const int8 Knight = 3;
			const int8 Bishop = 4;
			const int8 Rook = 5;
			const int8 Queen = 6;

			const int8 White = 8;
			const int8 Black = 16;

			const int8 ClassMask = 0x7;
		}

		const int8 Promotions[4]{ Piece::Queen, Piece::Rook, Piece::Knight, Piece::Bishop };

		namespace
		{
			struct Initialiser
			{
				static std::map<char, int8> ConstructMap()
				{
					std::map<char, int8> map;

					map['k'] = Piece::King;
					map['p'] = Piece::Pawn;
					map['n'] = Piece::Knight;
					map['b'] = Piece::Bishop;
					map['r'] = Piece::Rook;
					map['q'] = Piece::Queen;

					return map;
				}
			};
		} //Anonymous namespace to initialise the PieceTypeFromSymbol map

		const std::map<char, int8> PieceTypeFromSymbol = Initialiser::ConstructMap();

		static int8 NumSquaresToEdge[64][8];
		static void PrecomputeMoveData()
		{
			for (int8 file = 0; file < 8; file++)
			{
				for (int8 rank = 0; rank < 8; rank++)
				{
					int8 numNorth = 7 - rank;
					int8 numSouth = rank;
					int8 numWest = file;
					int8 numEast = 7 - file;

					int8 squareIdx = rank * 8 + file;

					NumSquaresToEdge[squareIdx][0] = numNorth;
					NumSquaresToEdge[squareIdx][1] = numSouth;
					NumSquaresToEdge[squareIdx][2] = numWest;
					NumSquaresToEdge[squareIdx][3] = numEast;
					NumSquaresToEdge[squareIdx][4] = std::min(numNorth, numWest);
					NumSquaresToEdge[squareIdx][5] = std::min(numSouth, numEast);
					NumSquaresToEdge[squareIdx][6] = std::min(numNorth, numEast);
					NumSquaresToEdge[squareIdx][7] = std::min(numSouth, numWest);
				}
			}
		}
	}
}