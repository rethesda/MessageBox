#pragma once

#include <functional>
#include <future>
#include <span>

namespace SkyrimScripting {

    class SkyrimMessageBox {
        class MessageBoxResultCallback : public RE::IMessageBoxCallback {
            std::function<void(uint32_t)> _callback;

        public:
            ~MessageBoxResultCallback() override {}
            MessageBoxResultCallback(std::function<void(uint32_t)> callback) : _callback(callback) {}
            void Run(RE::IMessageBoxCallback::Message message) override { _callback(static_cast<uint32_t>(message)); }
        };

    public:
        static void Show(const std::string& bodyText, std::span<std::string> buttonTextValues,
                         std::function<void(uint32_t)> callback) {
            auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
            auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
            auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStringHolder->messageBoxData);
            auto* messagebox = factory->Create();
            RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback =
                RE::make_smart<MessageBoxResultCallback>(callback);

            messagebox->callback = messageCallback;
            messagebox->bodyText = bodyText;
            for (auto text : buttonTextValues) messagebox->buttonText.push_back(text.c_str());
            messagebox->QueueMessage();
        }

        static std::future<uint32_t> ShowAsync(const std::string& bodyText, std::span<std::string> buttonTextValues) {
            auto promisePtr = std::make_shared<std::promise<uint32_t>>();
            std::future<uint32_t> future = promisePtr->get_future();
            Show(bodyText, buttonTextValues, [promisePtr](uint32_t result) { promisePtr->set_value(result); });
            return future;
        }
    };

    void ShowMessageBox(const std::string& bodyText, std::span<std::string> buttonTextValues,
                        std::function<void(uint32_t)> callback) {
        SkyrimMessageBox::Show(bodyText, buttonTextValues, callback);
    }
}
