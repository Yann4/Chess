// Fill out your copyright notice in the Description page of Project Settings.


#include "Test/ChessUnitTests.h"

void RunSomeSetup()
{

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ChessUnitTests, "TestGroup.TestSubgroup.Placeholder Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool ChessUnitTests::RunTest(const FString& Parameters)
{
	RunSomeSetup();
	// Make the test pass by returning true, or fail by returning false.
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ChessFailureTest, "TestGroup.TestSubgroup.Placeholder Test 2", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool ChessFailureTest::RunTest(const FString& Parameters)
{
	RunSomeSetup();
	// Make the test pass by returning true, or fail by returning false.
	return false;
}