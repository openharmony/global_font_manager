/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef FONT_MANAGER_FUZZ_DATA_H
#define FONT_MANAGER_FUZZ_DATA_H

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * 以下是生成输入参数
 */
int32_t NewInt32(const uint8_t* data, size_t size);

std::string NewString(const uint8_t* data, size_t size);

#endif
