// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvLeaf_Image.cpp

#include "Widgets/Composite/InvLeaf_Image.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"

void UInvLeaf_Image::SetImage(UTexture2D* Texture) const
{
	Image_Icon->SetBrushFromTexture(Texture);
}

void UInvLeaf_Image::SetImageSize(const FVector2D& Size) const
{
	Image_Icon->SetDesiredSizeOverride(Size);
}

FVector2D UInvLeaf_Image::GetImageSize() const
{
	return Image_Icon->GetDesiredSize();
}

void UInvLeaf_Image::SetBoxSize(const FVector2D& Size) const
{
	SizeBox_Icon->SetWidthOverride(Size.X);
	SizeBox_Icon->SetHeightOverride(Size.Y);
}
