// Fill out your copyright notice in the Description page of Project Settings.

#include "Board.h"
#include <assert.h>

namespace Chess
{
	using namespace Constants;

	Board::Board():
		BoardState(StandardStartFEN)
	{
		PrecomputeMoveData();
	}

	bool Board::MakeMove(Move& move)
	{
		if (IsValidMove(move))
		{
			BoardState.UpdateBoard(move);

			BoardState.EnPassentTarget = move.EnPassentTarget;

			BoardState.UpdateThreatMaps();

			BoardState.ColourToMove = BoardState.ColourToMove == Piece::White ? Piece::Black : Piece::White;
			return true;
		}

		return false;
	}

	bool Board::IsValidMove(Move& move) const
	{
		std::vector<Move> validMoves = MoveGeneration::GenerateMoves(BoardState, BoardState.ColourToMove);

		int8 king = Constants::DEFAULT;
		for (int8 square = 0; square < 64; square++)
		{
			if (Utils::IsType(BoardState.Squares[square], Piece::King) && Utils::IsColour(BoardState.Squares[square], BoardState.ColourToMove))
			{
				king = square;
				break;
			}
		}

		assert(king != Constants::DEFAULT);
		for (int idx = validMoves.size() - 1; idx >= 0; idx--)
		{
			if (Board::DoesMoveExposeKing(BoardState, validMoves[idx], king))
			{
				validMoves.erase(validMoves.begin() + idx);
			}
		}

		auto validatedMove = std::find(validMoves.begin(), validMoves.end(), move);
		bool moveIsValid = validatedMove != validMoves.end();
		if (moveIsValid)
		{
			move = *validatedMove;
		}

		return moveIsValid;
	}

	void State::UpdateBoard(const Move& move)
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
	}

	std::vector<Move> MoveGeneration::GenerateMoves(const State& board, int8 colour, bool calculateThreat /*= false*/)
	{
		std::vector<Move> moves;
		for (int8 startSquare = 0; startSquare < 64; startSquare++)
		{
			int8 piece = board.Squares[startSquare];

			if (Utils::IsColour(piece, colour))
			{
				if (Utils::IsSlidingPiece(piece))
				{
					GenerateSlidingMoves(board, startSquare, moves);
				}

				if (Utils::IsType(piece, Piece::Knight))
				{
					GenerateKnightMoves(board, startSquare, moves);
				}

				if (Utils::IsType(piece, Piece::Pawn))
				{
					if (calculateThreat)
					{
						GeneratePawnAttacks(board, startSquare, moves, calculateThreat);
					}
					else
					{
						GeneratePawnMoves(board, startSquare, moves);
						GeneratePawnAttacks(board, startSquare, moves);
					}
				}

				if (Utils::IsType(piece, Piece::King))
				{
					GenerateKingMoves(board, startSquare, moves);
				}
			}
		}

		return moves;
	}

	void MoveGeneration::GenerateSlidingMoves(const State& state, int8 startSquare, std::vector<Move>& moves)
	{
		int8 piece = state.Squares[startSquare];
		int8 friendlyColour = Utils::GetColour(piece);
		int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

		bool isRook = Utils::IsType(piece, Piece::Rook);
		int8 startDirIdx = Utils::IsType(piece, Piece::Bishop) ? 4 : 0;
		int8 endDirIdx = isRook ? 4 : 8;

		for (int8 directionIndex = startDirIdx; directionIndex < endDirIdx; directionIndex++)
		{
			for (int8 n = 0; n < NumSquaresToEdge[startSquare][directionIndex]; n++)
			{
				int8 targetSquare = startSquare + DirectionOffsets[directionIndex] * (n + 1);
				int8 pieceOnTargetSquare = state.Squares[targetSquare];

				if (Utils::IsColour(pieceOnTargetSquare, friendlyColour))
				{
					break;
				}

				if (isRook)
				{
					int8 prevented = Utils::RankIndex(0) ? Castling::Queenside : Castling::Kingside;
					moves.push_back(Move::CreateMove(startSquare, targetSquare, friendlyColour, prevented));
				}
				else
				{
					moves.push_back(Move::CreateMove(startSquare, targetSquare, friendlyColour));
				}

				if (Utils::IsColour(pieceOnTargetSquare, enemyColour))
				{
					break;
				}
			}
		}
	}

	void MoveGeneration::GenerateKnightMoves(const State& state, int8 startSquare, std::vector<Move>& moves)
	{
		const int8 KnightOffsets[8] = { 15, 17, -17, -15, 10, -6, 6, -10 };

		int8 piece = state.Squares[startSquare];
		int8 friendlyColour = Utils::GetColour(piece);
		int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

		for (int knightMoveIdx = 0; knightMoveIdx < 8; knightMoveIdx++)
		{
			int8 targetSquare = startSquare + KnightOffsets[knightMoveIdx];
			if (targetSquare < 0 || targetSquare >= 64)
			{
				continue;
			}

			if (Utils::IsColour(state.Squares[targetSquare], friendlyColour))
			{
				continue;
			}

			moves.push_back(Move::CreateMove(startSquare, targetSquare, friendlyColour));
		}
	}

	void MoveGeneration::GeneratePawnMoves(const State& state, int8 startSquare, std::vector<Move>& moves)
	{
		const int8 PawnOffsets[2][2]
		{
			{ 8, 16 }, //White
			{-8, -16}  //Black
		};

		int8 piece = state.Squares[startSquare];
		int8 colour = Utils::GetColour(piece);
		int8 colourIdx = Utils::IsColour(piece, Piece::White) ? 0 : 1;
		int8 startingRank = Utils::IsColour(piece, Piece::White) ? 1 : 6;
		int8 backRank = Utils::IsColour(piece, Piece::White) ? 7 : 0;
		int MovesAvailable = Utils::RankIndex(startSquare) == startingRank ? 2 : 1;

		for (int moveIdx = 0; moveIdx < MovesAvailable; moveIdx++)
		{
			int8 targetSquare = startSquare + PawnOffsets[colourIdx][moveIdx];
			if (state.Squares[targetSquare] != Piece::None)
			{
				break;
			}

			if (Utils::RankIndex(targetSquare) == backRank)
			{
				for (int8 promo : Constants::Promotions)
				{
					moves.push_back(Move::CreatePromotionMove(startSquare, targetSquare, colour, promo));
				}
			}
			else
			{
				if (moveIdx == 1)
				{
					moves.push_back(Move::CreateEnPassentMove(startSquare, targetSquare, colour));
				}
				else
				{
					moves.push_back(Move::CreateMove(startSquare, targetSquare, colour));
				}
			}
		}
	}

	void MoveGeneration::GeneratePawnAttacks(const State& state, int8 startSquare, std::vector<Move>& moves, bool calculateThreat)
	{
		const int8 PawnOffsets[2][3]
		{
			{ 7, 9 , 8}, //White
			{ -9, -7, -8 } //Black
		};

		int8 piece = state.Squares[startSquare];
		int8 colour = Utils::GetColour(piece);
		int8 colourIdx = Utils::IsColour(piece, Piece::White) ? 0 : 1;
		int8 enemyColour = Utils::IsColour(piece, Piece::White) ? Piece::Black : Piece::White;
		int8 file = Utils::FileIndex(startSquare);

		for (int moveIdx = 0; moveIdx < 2; moveIdx++)
		{
			//Make sure pawns on the 0th file can't attack left & the 7th can't attack right
			if ((file == 0 && moveIdx == 0) || file == 7 && moveIdx == 1)
			{
				continue;
			}

			int8 targetSquare = startSquare + PawnOffsets[colourIdx][moveIdx];

			int8 passentPawn = state.EnPassentTarget + PawnOffsets[colourIdx == 0 ? 1 : 0][2];
			if (state.EnPassentTarget == targetSquare && Utils::IsColour(state.Squares[passentPawn], enemyColour))
			{
				moves.push_back(Move::CreateEnPassentCapture(startSquare, targetSquare, colour, passentPawn, state.EnPassentTarget));
				continue;
			}

			if (calculateThreat || Utils::IsColour(state.Squares[targetSquare], enemyColour))
			{
				moves.push_back(Move::CreateMove(startSquare, targetSquare, colour));
			}
		}
	}

	void MoveGeneration::GenerateKingMoves(const State& state, int8 startSquare, std::vector<Move>& moves)
	{
		int8 piece = state.Squares[startSquare];
		int8 friendlyColour = Utils::GetColour(piece);
		int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

		for (int8 directionIndex = 0; directionIndex < 8; directionIndex++)
		{
			int8 targetSquare = startSquare + DirectionOffsets[directionIndex];
			int8 pieceOnTargetSquare = state.Squares[targetSquare];

			if (targetSquare < 0 || targetSquare >= 64)
			{
				continue;
			}

			if (Utils::IsColour(pieceOnTargetSquare, friendlyColour))
			{
				continue;
			}

			if (!state.IsSquareThreatened(targetSquare, friendlyColour))
			{
				moves.push_back(Move::CreateMove(startSquare, targetSquare, friendlyColour, Castling::Kingside | Castling::Queenside));
			}
		}

		int8 castleAvailability = friendlyColour == Piece::White ? state.WhiteCastleAvailable : state.BlackCastleAvailable;
		if ((castleAvailability & Castling::Kingside) == Castling::Kingside)
		{
			int8 startIdx = friendlyColour == Piece::White ? 5 : 61;
			int8 endIdx = friendlyColour == Piece::White ? 6 : 62;
			bool castlingBlocked = false;
			for (int idx = startIdx; idx < endIdx; idx++)
			{
				if (state.Squares[idx] != Piece::None || state.IsSquareThreatened(idx, friendlyColour))
				{
					castlingBlocked = true;
					break;
				}
			}

			if (!castlingBlocked)
			{
				moves.push_back(Move::CreateCastlingMove(friendlyColour, Castling::Kingside));
			}
		}

		if (((castleAvailability & Castling::Queenside) == Castling::Queenside))
		{
			int8 startIdx = friendlyColour == Piece::White ? 3 : 59;
			int8 endIdx = friendlyColour == Piece::White ? 2 : 57;
			bool castlingBlocked = false;

			for (int idx = endIdx; idx < startIdx; idx++)
			{
				if (state.Squares[idx] != Piece::None || state.IsSquareThreatened(idx, friendlyColour))
				{
					castlingBlocked = true;
					break;
				}
			}

			if (!castlingBlocked)
			{
				moves.push_back(Move::CreateCastlingMove(friendlyColour, Castling::Queenside));
			}
		}
	}

	bool Board::DoesMoveExposeKing(const State& state, const Move& move, int8 king)
	{
		State afterMove(state);
		int8 colour = state.ColourToMove;

		afterMove.UpdateBoard(move);
		afterMove.UpdateThreatMaps();

		int8 kingSquare = Utils::IsType(state.Squares[move.StartSquare], Piece::King) ? move.TargetSquare : king;
		return afterMove.IsSquareThreatened(kingSquare, colour);
	}

	State::State(const std::string& fen) :
		WhiteThreatMap(*this, Constants::Piece::White), BlackThreatMap(*this, Constants::Piece::Black)
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

	void ThreatMap::CalculateMap(const State& board)
	{
		Map = 0;
		for (const Move& move : MoveGeneration::GenerateMoves(board, Colour, true))
		{
			if (move.Castle == Castling::None)
			{
				SetThreatened(move.TargetSquare);
			}
		}
	}
}