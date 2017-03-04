// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "ProceduralNameGenerator.generated.h"

USTRUCT()
struct FMarkovData : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markov")
	FText MarkovText;
};

USTRUCT()
struct FMarkovCharacter
{
	GENERATED_BODY()
public:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markov")
	TCHAR Letter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markov")
	uint8 Instances;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markov")
	float Probability;

	FORCEINLINE bool operator <(const FMarkovCharacter& c) const
	{
		return Probability < c.Probability;
	}
	FORCEINLINE bool operator >(const FMarkovCharacter& c) const
	{
		return Probability < c.Probability;
	}
};

/**
 *
 */
UCLASS()
class POLYGONALMAPGENERATOR_API UProceduralNameGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//UFUNCTION(BlueprintCallable, Category = "Procedural|Markov Chain")
	//static TArray<FText> ConvertRiverDataToTextArray()
	UFUNCTION(BlueprintCallable, Category = "Procedural|Markov Chain")
	static FString GenerateRandomWord(TArray<FText> Words, FRandomStream RandomGenerator, uint8 WordOrder = 2, uint8 MinLength = 3, uint8 MaxLength = 16, bool bConvertToTitleCase = true, bool bMatchWordPattern = false);
	UFUNCTION(BlueprintCallable, Category = "Procedural|Markov Chain")
	static FString UppercaseWords(FString value);

private:
	static uint8 Order;
	static TMap<FString, TArray<FMarkovCharacter>> Chains;
	static TSet<FString> WordPatterns;

	static bool IsVowel(TCHAR ch);
	static FString GetWordPattern(FString word);
	static void IdentifyWordPattern(FString word);
	static void AddCharacter(FString key, TCHAR ch);
	static void ProcessWord(FString word);
	static void AnalyzeWord(FString word);
	static void CalculateProbability();
	static void AnalyzeWords(TArray<FString> words, uint8 order);
	static FMarkovCharacter* GetCharacterByProbability(FString key, float probability);
	static FString GenerateRandomWord(uint8 minLength, uint8 maxLength, FRandomStream RandomGenerator, bool bConvertToTitleCase);
};