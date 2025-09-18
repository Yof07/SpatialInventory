// Copyright Epic Games, Inc. All Rights Reserved.
// Widgets/Inventory.h
#pragma once

#include "Modules/ModuleManager.h"

/*
 * 声明一个自定义的日志分类，为库存系统创建独立的日志通道，便于分类管理和过滤日志
 * LogInventory：自定义日志分类名，前缀Log是UE的命名惯例
 * Log：日志详细级别，控制日志输出的默认过滤级别，Log及以上(Display、Warning、Error)的才会被输出(决定“哪些日志默认显示”)
 * All：控制哪些日志级别被默认编译到代码中(决定“哪些日志代码实际存在”)，在性能敏感的场景通常会设置为较高级别
 */
DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

class FInventoryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
