// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/ItemDescription/InvItemDescription.cpp

#include "Widgets/ItemDescription/InvItemDescription.h"

#include "Components/SizeBox.h"

FVector2D UInvItemDescription::GetBoxSize() const
{
	return SizeBox->GetDesiredSize();
}
