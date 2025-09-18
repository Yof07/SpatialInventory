// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvLeaf_Image.h
#pragma once

#include "CoreMinimal.h"
#include "InvLeaf.h"
#include "InvLeaf_Image.generated.h"

class USizeBox;
class UImage;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInvLeaf_Image : public UInvLeaf
{
	GENERATED_BODY()

public:
	void SetImage(UTexture2D* Texture) const;
	void SetImageSize(const FVector2D& Size) const;
	FVector2D GetImageSize() const;
	
	void SetBoxSize(const FVector2D& Size) const;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Icon;
};
