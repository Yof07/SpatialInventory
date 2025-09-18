// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvLeaf.cpp

#include "Widgets/Composite/InvLeaf.h"

void UInvLeaf::ApplyFunction(FuncType Function)
{
	// 把自己传入当作参数，一般是显示数值等
	Function(this);
}
