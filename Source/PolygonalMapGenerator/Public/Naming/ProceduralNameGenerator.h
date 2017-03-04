/*
The MIT License (MIT)

Copyright (c) 2015 Dave Carlile
Modifications made in 2017 Jay Stevens

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "ProceduralNameGenerator.generated.h"

USTRUCT(BlueprintType)
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
	static FString GenerateRandomWord(uint8 minLength, uint8 maxLength, FRandomStream& RandomGenerator, bool bConvertToTitleCase);
};