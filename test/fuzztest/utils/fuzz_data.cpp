/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "fuzz_data.h"

#include <algorithm>
#include <iterator>

/**
 * 以下是生成输入参数
 */
std::string NewString(const uint8_t* data, size_t size)
{
    return std::string(reinterpret_cast<const char*>(data), size);
}

int32_t NewInt32(const uint8_t* data, size_t size)
{
    uint32_t val = 0;
    if (size >= sizeof(uint32_t)) {
        val |= *reinterpret_cast<const uint32_t*>(data);
    } else {
        for (size_t i = 0; i < size; ++i) {
            val |= (data[i] << i);
        }
    }
    return static_cast<int32_t>(val);
}
