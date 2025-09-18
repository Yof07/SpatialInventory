// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvLeaf_Text.cpp

#include "Widgets/Composite/InvLeaf_Text.h"

#include "Components/TextBlock.h"

void UInvLeaf_Text::SetText(const FText& Text) const
{
	Text_LeafText->SetText(Text);
}

void UInvLeaf_Text::NativePreConstruct()
{
	Super::NativePreConstruct();

	FSlateFontInfo FontInfo = Text_LeafText->GetFont();
	FontInfo.Size = FontSize;
	
	Text_LeafText->SetFont(FontInfo);
}
