// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceActor.generated.h"

UENUM(BlueprintType)
enum class Class : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	King = 1 UMETA(DisplayName = "King"),
	Pawn = 2 UMETA(DisplayName = "Pawn"),
	Knight = 3 UMETA(DisplayName = "Knight"),
	Bishop = 4 UMETA(DisplayName = "Bishop"),
	Rook = 5 UMETA(DisplayName = "Rook"),
	Queen = 6 UMETA(DisplayName = "Queen"),
};

UCLASS()
class CHESS_API APieceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APieceActor();

	UFUNCTION(BlueprintNativeEvent)
	void SetPieceType(Class piece, bool black);

	UFUNCTION(BlueprintNativeEvent)
	void TakePiece();

	UFUNCTION()
	void MoveTo(const FVector& square);
protected:
	UPROPERTY(BlueprintReadOnly)
	Class PieceType;

	UPROPERTY(BlueprintReadOnly)
	bool Black;

	UPROPERTY(BlueprintReadOnly)
	bool Taken = false;
};
