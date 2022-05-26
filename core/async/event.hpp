#pragma once

#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <condition_variable>
#include <typeindex>
#include <type_traits>
#include "print.hpp"

class EventPipe {
private:
	struct EventType {
		virtual void DispatchIfPresent() = 0;

		virtual void Push(void *e) = 0;
	};
private:
	std::unordered_map<std::type_index, std::unique_ptr<EventType>> m_EventTypes;

	std::mutex m_Lock;
	std::condition_variable m_CondVar;
public:
	
	template<typename T>
	void RegisterEventType(std::function<void(T)> callback){ 
		std::type_index index(typeid(T));

		assert(m_EventTypes.find(index) == m_EventTypes.end());

		struct Event: EventType {
			std::function<void(T)> Callback;
			std::queue<T> Events;

			Event(std::function<void(T)> callback):
				Callback(callback)
			{}

			void DispatchIfPresent()override {
				while (!Events.empty()) {
					Callback(std::move(Events.back()));
					Events.pop();
				}
			}

			void Push(void* fe)override{
				T *e = reinterpret_cast<T*>(fe);

				Events.push(std::move(*e));
			}
		};

		m_EventTypes.emplace(index, std::make_unique<Event>(callback));
	}

	template<typename T>
	void PushEvent(T e){
		{
			std::unique_lock<std::mutex> guard(m_Lock);

			std::type_index index(typeid(T));

			assert(m_EventTypes.find(index) != m_EventTypes.end());

			std::unique_ptr<EventType> &type = m_EventTypes.find(index)->second;

			type->Push(&e);
			Println("[EventPipe]: Pushed %", typeid(T).name());
		}

		m_CondVar.notify_one();
	}
	
	void WaitAndDispath(){
		std::unique_lock<std::mutex> guard(m_Lock);

		m_CondVar.wait(guard);

		for (auto& event_type : m_EventTypes)
			event_type.second->DispatchIfPresent();
	}
};