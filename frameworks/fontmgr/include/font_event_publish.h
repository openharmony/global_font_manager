/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FONT_EVENT_PUBLISH_H
#define FONT_EVENT_PUBLISH_H

#include <string>

#include "font_define.h"

namespace OHOS {
namespace Global {
namespace FontManager {
enum FontEventType {
    INSTALL = 0,
    UNINSTALL = 1
};

class FontEventPublish {
public:
    static bool PublishFontUpdate(const FontEventType eventType, const std::string &formatName);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_EVENT_PUBLISH_H