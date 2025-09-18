// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvCompositeBase.cpp

#include "Widgets/Composite/InvCompositeBase.h"

void UInvCompositeBase::Collapse()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInvCompositeBase::Expand()
{
	SetVisibility(ESlateVisibility::Visible);
}
