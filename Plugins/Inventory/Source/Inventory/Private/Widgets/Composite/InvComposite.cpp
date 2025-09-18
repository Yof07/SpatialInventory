// Fill out your copyright notice in the Description page of Project Settings.
// Widgets/Composite/InvComposite.cpp

#include "Widgets/Composite/InvComposite.h"

#include "Blueprint/WidgetTree.h"

void UInvComposite::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 跟踪所有子组件
	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UInvCompositeBase* Composite = Cast<UInvCompositeBase>(Widget); IsValid(Composite))
		{
			Children.Add(Composite);
			Composite->Collapse();
		}
	});
}

void UInvComposite::ApplyFunction(FuncType Function)
{
	for (auto& Child : Children)
	{
		Child->ApplyFunction(Function);
	}
}

void UInvComposite::Collapse()
{
	for (auto& Child : Children)
	{
		Child->Collapse(); // 子节点自己折叠
	}
}
