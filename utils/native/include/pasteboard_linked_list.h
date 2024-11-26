/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_LINKED_LIST_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_LINKED_LIST_H

#include <functional>
#include <mutex>

namespace OHOS {
namespace MiscServices {

template <typename T>
struct ListNode {
    explicit ListNode(const T &val = {}) : value(val), next(nullptr)
    {
    }

    T value;
    ListNode<T> *next;
};

template <typename T>
class LinkedList {
public:
    LinkedList()
    {
        head_ = new ListNode<T>();
    }

    ~LinkedList()
    {
        Clear();
        if (head_) {
            delete head_;
            head_ = nullptr;
        }
    }

    void Clear() noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == nullptr) {
            return;
        }

        ListNode<T> *iter = head_->next;
        ListNode<T> *next = nullptr;
        while (iter) {
            next = iter->next;
            delete iter;
            iter = next;
        }
        head_->next = nullptr;
    }

    void InsertFront(const T &value) noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == nullptr) {
            return;
        }

        ListNode<T> *newNode = new ListNode<T>(value);
        newNode->next = head_->next;
        head_->next = newNode;
    }

    void InsertTail(const T &value) noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == nullptr) {
            return;
        }

        ListNode<T> *newNode = new ListNode<T>(value);
        ListNode<T> *iter = head_;
        while (iter->next) {
            iter = iter->next;
        }
        iter->next = newNode;
    }

    void RemoveIf(std::function<bool(const T&)> func) noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == nullptr) {
            return;
        }

        ListNode<T> *iter = head_->next;
        ListNode<T> *prev = head_;
        while (iter) {
            if (func(iter->value)) {
                prev->next = iter->next;
                delete iter;
                iter = prev->next;
            } else {
                prev = iter;
                iter = iter->next;
            }
        }
    }

    bool FindExist(const T &value) noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == nullptr) {
            return false;
        }

        ListNode<T> *iter = head_->next;
        while (iter) {
            if (iter->value == value) {
                return true;
            }
            iter = iter->next;
        }
        return false;
    }

    bool FindExist(std::function<bool(const T&)> func) noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == nullptr) {
            return false;
        }

        ListNode<T> *iter = head_->next;
        while (iter) {
            if (func(iter->value)) {
                return true;
            }
            iter = iter->next;
        }
        return false;
    }

    void ForEach(std::function<void(const T&)> func) noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == nullptr) {
            return;
        }

        ListNode<T> *iter = head_->next;
        while (iter) {
            func(iter->value);
            iter = iter->next;
        }
    }

private:
    ListNode<T> *head_;
    std::mutex mutex_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_LINKED_LIST_H
