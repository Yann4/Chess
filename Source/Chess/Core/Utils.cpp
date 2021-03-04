#include "Utils.h"

namespace Chess
{
	using namespace Constants;

	std::string Utils::PieceName(int8 piece)
	{
		std::string pieceName = IsColour(piece, Piece::Black) ? "Black " : "White ";
		if (IsType(piece, Piece::Pawn))
		{
			pieceName += "Pawn";
		}
		else if (IsType(piece, Piece::Rook))
		{
			pieceName += "Rook";
		}
		else if (IsType(piece, Piece::Bishop))
		{
			pieceName += "Bishop";
		}
		else if (IsType(piece, Piece::Knight))
		{
			pieceName += "Knight";
		}
		else if (IsType(piece, Piece::King))
		{
			pieceName += "King";
		}
		else if (IsType(piece, Piece::Queen))
		{
			pieceName += "Queen";
		}

		return pieceName;
	}

	std::string Utils::AlgebraicName(int8 piece)
	{
		if (IsType(piece, Piece::Pawn))
		{
			return "";
		}
		else if (IsType(piece, Piece::Rook))
		{
			return "R";
		}
		else if (IsType(piece, Piece::Bishop))
		{
			return "B";
		}
		else if (IsType(piece, Piece::Knight))
		{
			return "N";
		}
		else if (IsType(piece, Piece::King))
		{
			return "K";
		}
		else if (IsType(piece, Piece::Queen))
		{
			return "Q";
		}

		return "ERROR";
	}
}