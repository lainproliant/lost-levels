#include "lost_levels/event.h"
#include "lain/testing.h"

using namespace std;
using namespace lain;
using namespace lain::testing;
using namespace lost_levels;

class CustomEvent : public Event {
public:
   static const EventType TYPE;

   CustomEvent(const string& data) : Event(TYPE), data(data) { }
   const string& get_custom_data() const { return data; }

   virtual CustomEvent* clone() const {
      return new CustomEvent(data);
   }

private:
   string data;
};

const EventType CustomEvent::TYPE = declareEvent("customEvent");

int main() {
   return TestSuite("lost_levels event tests")
      .die_on_signal(SIGSEGV)
      .test("Event-001: Basic EventBus test", [&]()->bool {
         bool called = false;
         const EventType TEST_EVENT = declareEvent("testEvent");
         EventBus bus;

         bus.subscribe(TEST_EVENT, [&called](const Event& event) {
            cout << "Closure received event "
                 << event.get_type().get_name() << endl;
            called = true;
         });

         bus.publish(Event(TEST_EVENT));
         bus.process_events();
         return called;
      })
      .test("Event-002: Custom event types test", [&]()->bool {
         bool called = false;
         EventBus bus;

         bus.subscribe(CustomEvent::TYPE, [&called](const Event& event) {
            cout << "Event type: " << event.get_type().get_name() << endl;
            const CustomEvent& customEvent = static_cast<const CustomEvent&>(event);
            cout << "Closure received event "
                 << customEvent.get_type().get_name()
                 << " with data \""
                 << customEvent.get_custom_data()
                 << "\"" << endl;
            called = true;
         });

         bus.publish(CustomEvent("Hello World!"));
         bus.process_events();
         return called;
      })
      .run();
}

