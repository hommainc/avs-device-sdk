/*
 * ObservableMessageRequest.cpp
 *
 * Copyright 2016-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "Integration/ObservableMessageRequest.h"

#include <iostream>

namespace alexaClientSDK {
namespace integration {

using namespace avsCommon::avs;
using namespace avsCommon::avs::attachment;

ObservableMessageRequest::ObservableMessageRequest(
        const std::string & jsonContent,
        std::shared_ptr<AttachmentReader> attachmentReader) :
        MessageRequest{jsonContent, attachmentReader}, m_sendMessageStatus(MessageRequest::Status::PENDING) {
}

void ObservableMessageRequest::onSendCompleted(MessageRequest::Status sendMessageStatus) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sendMessageStatus = sendMessageStatus;
    m_wakeTrigger.notify_all();
}

MessageRequest::Status ObservableMessageRequest::getSendMessageStatus() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sendMessageStatus;
}

bool ObservableMessageRequest::waitFor(
        const MessageRequest::Status sendMessageStatus, const std::chrono::seconds duration) {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_wakeTrigger.wait_for(lock, duration, [this, sendMessageStatus]() {
        return sendMessageStatus == m_sendMessageStatus;
    });
}

void ObservableMessageRequest::onExceptionReceived(const std::string & exceptionMessage) {
    std::cout << "Received exception message:" << exceptionMessage << std::endl;
}

} // namespace integration
} // namespace alexaClientSDK