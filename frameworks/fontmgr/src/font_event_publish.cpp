/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "font_event_publish.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_support.h>
#include <want.h>

namespace OHOS {
namespace Global {
namespace FontManager {
static const std::string FONT_UPDATE_FOR_POLICY = "usual.event.FONT_UPDATE_FOR_POLICY";
static const std::string FONT_EVENT_TYPE = "eventType";
static const std::string FONT_EVENT_FONT_NAMES = "fontFullNames";

bool FontEventPublish::PublishFontUpdate(const FontEventType eventType, const std::string &fullName)
{
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    OHOS::AAFwk::Want updateWant;
    updateWant.SetAction(FONT_UPDATE_FOR_POLICY);
    updateWant.SetParam(FONT_EVENT_TYPE, eventType);
    updateWant.SetParam(FONT_EVENT_FONT_NAMES, fullName);

    OHOS::EventFwk::CommonEventData event(updateWant);
    return OHOS::EventFwk::CommonEventManager::PublishCommonEvent(event, publishInfo, nullptr);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
