/*
 * Event: Standard event types.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Friday, Jan 2 2015
 */
#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <deque>

#include "lain/exception.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   class Event;
   class EventType;
   class EventRegistry;

   typedef function<void(const Event&)> EventFunction;

   class UnknownEventTypeException : public Exception {
   public:
      using Exception::Exception;
   };

   /**
    * Event <<concrete base class>>
    *
    * An object encapsulating an instance of an event.
    *
    * Typically, consumers will extend Event to include
    * data specific to the event being published.
    */
   class Event {
   public:
      Event(const EventType& eventType) : eventType(eventType) { }
      virtual ~Event() { }

      const EventType& get_type() const { return eventType; }
      virtual Event* clone() const { return new Event(*this); }

   private:
      const EventType& eventType;
   };

   /**
    * EventType
    *
    * Uniquely identifies a particular type of event both
    * by common name and by id.
    *
    * Unique instances of this object can only be created
    * by the EventRegistry, though external code is free
    * to copy existing EventType objects.
    *
    * These objects are used to associate EventHandler
    * or closure objects to particular events in the
    * EventBus.
    */
   class EventType {
      friend class EventRegistry;

   public:
      EventType(const EventType& rhs) :
         EventType(rhs.eventName, rhs.eventId) { }

      virtual ~EventType() { }

      string get_name() const { return eventName; }
      int get_id() const { return eventId; }

   private:
      EventType(const string& eventName, int eventId) :
         eventName(eventName), eventId(eventId) { }

      string eventName;
      int eventId;
   };

   /**
    * EventRegistry
    *
    * A singleton object responsible for controlling the
    * creation and acquisition of EventType objects.
    *
    * This object insures that there is only one unique ID
    * per named event, and allows consumers to refer to
    * event types by name without incurring additional
    * runtime cost.
    */
   class EventRegistry {
   public:
      /**
       * Fetch or create the singleton instance.
       */
      static EventRegistry& get_instance() {
         static EventRegistry singleton;
         return singleton;
      }

      /**
       * Declare a new event, or fetch the ID for
       * an event which has already been declared.
       */
      const EventType& get_id(const string& eventName, bool createNew = true) {
         if (eventTypeMap.find(eventName) == eventTypeMap.end()) {
            if (createNew) {
               eventTypeMap.insert({eventName, EventType(eventName, ++next_id)});
            } else {
               throw UnknownEventTypeException(eventName);
            }
         }

         return eventTypeMap.find(eventName)->second;
      }

   private:
      EventRegistry() { }
      virtual ~EventRegistry() { }

      unordered_map<string, EventType> eventTypeMap;
      int next_id = 0;
   };

   /**
    * EventHandler <<interface>>
    *
    * An interface for objects which handle events.
    */
   class EventHandler : public enable_shared_from_this<EventHandler> {
   public:
      virtual ~EventHandler() { }

      virtual void handle_event(const Event& event) = 0;

      shared_ptr<EventHandler> get_shared_ptr() {
         return shared_from_this();
      }
   };

   /**
    * FunctionalEventHandler <<concrete class>>
    *    implements EventHandler
    *
    * A specialization of the EventHandler interface allowing
    * a user-defined closure to be invoked when an event
    * is published.
    */
   class FunctionalEventHandler : public EventHandler {
   public:
      FunctionalEventHandler(EventFunction closure) :
         closure(closure) { }

      virtual void handle_event(const Event& event) {
         closure(event);
      }

   private:
      EventFunction closure;
   };

   /**
    * EventBus
    *
    * Provides a centralized bus into which events can be published,
    * and to which EventHandler objects or closures can subscribe
    * for specific events.
    */
   class EventBus {
   public:
      EventBus() { }
      virtual ~EventBus() { }

      EventBus(const EventBus&) = delete;
      EventBus& operator=(const EventBus&) = delete;

      void publish(const string& eventName) {
         publish(Event(EventRegistry::get_instance().get_id(eventName)));
      }

      /**
       * Publish a copy of the given event to the bus.
       *
       * NOTE:
       * The r-value reference version of this method is preferred
       * for new events as it does not require copying.
       */
      void publish(Event& event) {
         events.push_front(shared_ptr<Event>(event.clone()));
      }

      /**
       * Publish a copy of the given event to the bus.
       *
       * NOTE:
       * The r-value reference version of this method is preferred
       * for new events as it does not require copying.
       */
      void publish(Event&& event) {
         events.push_front(shared_ptr<Event>(event.clone()));
      }

      void subscribe(const string& eventName, EventFunction closure) {
         subscribe(eventName, make_shared<FunctionalEventHandler>(closure));
      }

      void subscribe(const string& eventName, shared_ptr<EventHandler> handler) {
         subscribe(EventRegistry::get_instance().get_id(eventName), handler);
      }

      /**
       * Subscribe the given closure to an event on the bus.
       * The closure provided will be invoked when the event is
       * published and processed.
       *
       * The closure/function provided must satisfy the object
       * template "std::function<void(const Event& event)>".
       *
       * Example:
       * <code>
       *    bus.subscribe(CUSTOM_EVENT, [](const Event& event) {
       *       // do stuff
       *    });
       * </code>
       */
      void subscribe(const EventType& eventType, EventFunction closure) {
         subscribe(eventType, make_shared<FunctionalEventHandler>(closure));
      }

      /**
       * Subscribe the given EventHandler object to an event on the bus,
       * using the given smart pointer.  Use this method exclusively
       * if you are already managing your EventHandler object with a
       * shared pointer, to avoid double-free madness.
       */
      void subscribe(const EventType& eventType, shared_ptr<EventHandler> handler) {
         handlers.insert({eventType.get_id(), handler});
      }

      /**
       * Channel all of the events from this bus into another.
       */
      void channel(EventBus& bus) {
         for (auto event : events) {
            bus.events.push_front(event);
         }
      }

      /**
       * Process all of the events in the event queue.
       *
       * This method should be called once the consumer is ready for
       * event handlers to be processed.  Event handlers are not called
       * automatically, so this method must be called at some time
       * during the game loop.
       */
      void process_events() {
         while (! events.empty()) {
            shared_ptr<Event> event = events.back();
            auto range = handlers.equal_range(event->get_type().get_id());
            for (auto iter = range.first; iter != range.second; iter++) {
               iter->second->handle_event(*event);
            }

            events.pop_back();
         }
      }

   private:
      deque<shared_ptr<Event>> events;
      unordered_multimap<int, shared_ptr<EventHandler>> handlers;
   };

   /**
    * Declare the given event if it does not already exist.
    * Returns the EventType for the event named.
    *
    * Consumers should call this while they are loading to
    * avoid runtime overhead.
    */
   inline EventType declareEvent(const string& eventName) {
      return EventRegistry::get_instance().get_id(eventName);
   }
}

