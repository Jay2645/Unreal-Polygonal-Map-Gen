/*
The MIT License (MIT)

Copyright (c) 2015 Dave Carlile

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

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Misc/Char.h"
#include <string>
#include "ProceduralNameGenerator.h"

uint8 UProceduralNameGenerator::Order = 0;
TMap<FString, TArray<FMarkovCharacter>> UProceduralNameGenerator::Chains = TMap<FString, TArray<FMarkovCharacter>>();
TSet<FString> UProceduralNameGenerator::WordPatterns = TSet<FString>();

bool UProceduralNameGenerator::IsVowel(TCHAR ch)
{
	FString vowels = FString(TEXT("aeiou"));
	//FString extendedVowels = FString(TEXT("����������������������"));
	int32 index;
	return vowels.FindChar(ch, index);
}

FString UProceduralNameGenerator::GetWordPattern(FString word)
{
	// This takes a string and converts it to a pattern
	// The pattern is the ratio of vowels to consonants
	// As an example, "consonants" would be "cvccvcvccc"
	std::string pattern = "";

	TArray<TCHAR> wordArray = word.GetCharArray();
	for(int i = 0; i < wordArray.Num(); i++)
	{
		TCHAR ch = wordArray[i];
		pattern += IsVowel(ch) ? "v" : "c";
	}

	return FString(pattern.c_str());
}

void UProceduralNameGenerator::IdentifyWordPattern(FString word)
{
	// Determines the word's pattern and adds it
	// to the list of word patterns
	WordPatterns.Add(GetWordPattern(word));
}

void UProceduralNameGenerator::AddCharacter(FString key, TCHAR ch)
{
	TArray<FMarkovCharacter> chain;

	// Ensure that the dictionary contains the key
	if (Chains.Contains(key))
	{
		chain = Chains[key];
	}
	else
	{
		Chains.Add(key, chain);
	}

	// Check to see if it is already in this chain
	bool bFound = false;
	for (int i = 0; i < chain.Num(); i++)
	{
		if (chain[i].Letter == ch)
		{
			// We have a match!
			// Update the number of instances of this letter
			chain[i].Instances++;
			bFound = true;
			break;
		}
	}

	// If we didn't find a match, update the chain with a new letter
	if (!bFound)
	{
		FMarkovCharacter letter = FMarkovCharacter();
		letter.Letter = ch;
		letter.Instances = 1;
		chain.Add(letter);
	}
	
	// Update the main Chains array
	Chains[key] = chain;
}

void UProceduralNameGenerator::ProcessWord(FString word)
{
	std::string wordStr = "";
	// Convert to a std string
	// Begin with a few underscores
	for (int i = 0; i < Order; i++)
	{
		wordStr += '_';
	}
	// Actually convert the string
	for (int i = 0; i < word.Len(); i++)
	{
		wordStr += word[i];
	}
	// Terminate with an underscore
	wordStr += '_';

	// Add the keys to the Chains array
	for (int i = 0; i < wordStr.size() - Order; i++)
	{
		// Each key is a substring from the current index plus the order
		std::string key = wordStr.substr(i, Order);
		FString unrealKey = FString(key.c_str());
		// Now add each character to the array
		// We add Order to the index to compensate for the underscores we added
		AddCharacter(unrealKey, wordStr[i + Order]);
	}
}

void UProceduralNameGenerator::AnalyzeWord(FString word)
{
	IdentifyWordPattern(word);
	ProcessWord(word);
}

void UProceduralNameGenerator::CalculateProbability()
{
	TMap<FString, TArray<FMarkovCharacter>> newChains;

	for (TPair<FString, TArray<FMarkovCharacter>>& kvp : Chains)
	{
		if(kvp.Key.IsEmpty() || kvp.Value.Num() == 0)
		{
			continue;
		}
		float totalInstances = 0.0f;
		TArray<FMarkovCharacter> characterArray = kvp.Value;
		for (int i = 0; i < characterArray.Num(); i++)
		{
			totalInstances += characterArray[i].Instances;
		}
		for (int i = 0; i < characterArray.Num(); i++)
		{
			characterArray[i].Probability = characterArray[i].Instances / totalInstances;
		}
		characterArray.HeapSort();
		newChains.Add(kvp.Key, characterArray);
	}

	Chains.Empty();
	Chains = newChains;
}

void UProceduralNameGenerator::AnalyzeWords(TArray<FString> words, uint8 order)
{
	Order = order;

	Chains.Empty();
	WordPatterns.Empty();

	for(int i = 0; i < words.Num(); i++)
	{
		AnalyzeWord(words[i].ToLower());
	}

	CalculateProbability();
}

FMarkovCharacter* UProceduralNameGenerator::GetCharacterByProbability(FString key, float probability)
{
	TArray<FMarkovCharacter> chain;
	if (!Chains.Contains(key))
	{
		UE_LOG(LogWorldGen, Warning, TEXT("Chain does not contain %s!"), *key);
		return NULL;
	}
	else
	{
		chain = Chains[key];
	}

	float cumulative = 0;
	FMarkovCharacter* result = NULL;

	for (int i = 0; i < chain.Num(); i++)
	{
		cumulative += chain[i].Probability;
		if (probability < cumulative)
		{
			result = new FMarkovCharacter();
			result->Instances = chain[i].Instances;
			result->Letter = chain[i].Letter;
			result->Probability = chain[i].Probability;
			break;
		}
	}

	return result;
}

FString UProceduralNameGenerator::GenerateRandomWord(TArray<FText> Words, FRandomStream RandomGenerator, uint8 WordOrder, uint8 MinLength, uint8 MaxLength, bool bConvertToTitleCase, bool bMatchWordPattern)
{
	TArray<FString> wordStrings;
	wordStrings.SetNumZeroed(Words.Num());
	for(int i = 0; i < Words.Num(); i++)
	{
		wordStrings[i] = Words[i].ToString();
	}
	for(int i = 0; i < wordStrings.Num(); i++)
	{
		UE_LOG(LogWorldGen, Log, TEXT("%d: %s"), i, *wordStrings[i]);
	}
	AnalyzeWords(wordStrings, WordOrder);
	for (int i = 0; i < 64; i++)
	{
		FString word = GenerateRandomWord(MinLength, MaxLength, RandomGenerator, bConvertToTitleCase);
		UE_LOG(LogWorldGen, Log, TEXT("Generated word: %s"), *word);
		if (word.Len() < MinLength) continue;
		if (!bMatchWordPattern) return word;
		if (WordPatterns.Contains(GetWordPattern(word))) return word;
	}

	UE_LOG(LogWorldGen, Error, TEXT("Could not generate word!"));
	return FString();
}

FString UProceduralNameGenerator::GenerateRandomWord(uint8 minLength, uint8 maxLength, FRandomStream& RandomGenerator, bool bConvertToTitleCase)
{
	std::string result;

	result = "";
	std::string key = "";
	for (int i = 0; i < Order; i++)
	{
		key += '_';
	}

	while (result.length() < maxLength)
	{
		FMarkovCharacter* character = GetCharacterByProbability(FString(key.c_str()), RandomGenerator.GetFraction());
		char ch = character == NULL ? '_' : character->Letter;
		delete character;
		if (ch == '_') break;

		result += ch;
		key += ch;

		key = key.substr(key.length() - Order);
	}
	if (bConvertToTitleCase)
	{
		return UppercaseWords(FString(result.c_str()));
	}
	else
	{
		result[0] = TChar<TCHAR>::ToUpper(result[0]);
		return FString(result.c_str());
	}
}

FString UProceduralNameGenerator::UppercaseWords(FString value)
{
	TArray<TCHAR> charArray = value.GetCharArray();
	// Handle the first letter in the FString.
	if (charArray.Num() >= 1)
	{
		if (TChar<TCHAR>::IsLower(charArray[0]))
		{
			charArray[0] = TChar<TCHAR>::ToUpper(charArray[0]);
		}
	}
	// Scan through the letters, checking for spaces.
	// ... Uppercase the lowercase letters following spaces.
	for (int i = 1; i < charArray.Num(); i++)
	{
		if (charArray[i - 1] == ' ')
		{
			if (TChar<TCHAR>::IsLower(charArray[i]))
			{
				charArray[i] = TChar<TCHAR>::ToUpper(charArray[i]);
			}
		}
	}
	std::string finishedString = "";
	for (int i = 0; i < charArray.Num(); i++)
	{
		finishedString += charArray[i];
	}
	return FString(finishedString.c_str());
}
