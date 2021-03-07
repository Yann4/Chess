#include "State.h"
#include <assert.h>
#include <cctype>

#include "Utils.h"

namespace Chess
{
	using namespace Constants;

	State::State(const std::string& fen) :
		WhiteThreatMap(*this, Piece::White), BlackThreatMap(*this, Piece::Black)
	{
		memset(Squares, 0, 64);

		size_t endOfBoard = fen.find(' ');
		if (endOfBoard != std::string::npos)
		{
			std::string fenBoard = fen.substr(0, endOfBoard);
			int file = 0;
			int rank = 7;

			for (char symbol : fenBoard)
			{
				if (symbol == '/')
				{
					file = 0;
					rank--;
				}
				else
				{
					if (std::isdigit(symbol))
					{
						file += std::atoi(&symbol);
					}
					else
					{
						int colour = std::isupper(symbol) ? Piece::White : Piece::Black;
						int piece = PieceTypeFromSymbol.at(static_cast<char>(std::tolower(symbol)));
						Squares[rank * 8 + file] = colour | piece;
						file++;
					}
				}
			}

			std::string toPlay = fen.substr(endOfBoard + 1, 1);
			if (toPlay.find("w") != std::string::npos)
			{
				ColourToMove = Piece::White;
			}
			else
			{
				ColourToMove = Piece::Black;
			}

			size_t endOfCastling = fen.find(' ', endOfBoard + 3);
			std::string castling = fen.substr(endOfBoard + 3, endOfCastling - (endOfBoard + 3));

			BlackCastleAvailable = castling.find("k") != std::string::npos ? Castling::Kingside : Castling::None;
			BlackCastleAvailable |= castling.find("q") != std::string::npos ? Castling::Queenside : Castling::None;

			WhiteCastleAvailable = castling.find("K") != std::string::npos ? Castling::Kingside : Castling::None;
			WhiteCastleAvailable |= castling.find("Q") != std::string::npos ? Castling::Queenside : Castling::None;

			//Not currently being used
			size_t endOfEP = fen.find(' ', endOfCastling + 1);
			std::string enPassent = fen.substr(endOfCastling + 1, endOfEP - (endOfCastling + 1));

			size_t endOfHM = fen.find(' ', endOfEP + 1);
			int8 HalfMoveCounter = std::atoi(fen.substr(endOfEP + 1, endOfHM - (endOfEP + 1)).c_str());

			size_t endOfFM = fen.find(' ', endOfHM + 1);
			int8 FullMoveCounter = std::atoi(fen.substr(endOfHM + 1, endOfFM - (endOfHM + 1)).c_str());
		}
	}

	void State::Update(const Move& move)
	{
		Squares[move.TargetSquare] = Squares[move.StartSquare];
		Squares[move.StartSquare] = 0;

		if (move.PreventsCastling != Castling::None)
		{
			int8& updateCastling = Utils::IsColour(move.Colour, Piece::White) ? WhiteCastleAvailable : BlackCastleAvailable;
			updateCastling &= ~move.PreventsCastling;
		}

		if (move.SecondaryStart != -1)
		{
			if (move.SecondaryTarget != -1) //Target is -1 in case of en passent, a real value in the case of castling
			{
				Squares[move.SecondaryTarget] = Squares[move.SecondaryStart];
			}

			Squares[move.SecondaryStart] = 0;
		}

		if (move.Promote != Piece::None)
		{
			Squares[move.TargetSquare] = ColourToMove | move.Promote;
		}

		EnPassentTarget = move.EnPassentTarget;

		UpdateThreatMaps();

		ColourToMove = ColourToMove == Piece::White ? Piece::Black : Piece::White;
	}

	void State::UpdateThreatMaps()
	{
		WhiteThreatMap.CalculateMap(*this);
		BlackThreatMap.CalculateMap(*this);
	}

	bool State::IsSquareThreatened(int8 square, int8 friendlyColour) const
	{
		const ThreatMap& threats = Utils::IsColour(friendlyColour, Constants::Piece::White) ? BlackThreatMap : WhiteThreatMap;
		return threats.IsThreatened(square);
	}

	bool State::IsKingThreatened(int8 colour) const
	{
		int8 king = FindPiece(colour | Piece::King)[0];
		return IsSquareThreatened(king, colour);
	}

	std::vector<int8> State::FindPiece(int8 piece) const
	{
		std::vector<int8> pieceLocations;
		for (int idx = 0; idx < 64; idx++)
		{
			if (Squares[idx] == piece)
			{
				pieceLocations.push_back(idx);

				if ((Utils::IsType(piece, Piece::King) || Utils::IsType(piece, Piece::Queen)) || //There can be only one
					(pieceLocations.size() == 2 && (Utils::IsType(piece, Piece::Rook) || Utils::IsType(piece, Piece::Knight) || Utils::IsType(piece, Piece::Bishop))) ||
					(pieceLocations.size() == 8 && Utils::IsType(piece, Piece::Pawn)))
				{
					break;
				}
			}
		}

		return pieceLocations;
	}
}