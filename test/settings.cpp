#include "lost_levels/settings.h"
#include "lain/testing.h"

using namespace std;
using namespace lain;
using namespace lain::testing;
using namespace lost_levels;

int main(int argc, char** argv) {
   return TestSuite("lost_levels settings tests")
      .die_on_signal(SIGSEGV)
      .test("Settings-001: Loading settings from a file", [&]()->bool {
         Settings settings = Settings::load_from_file("settings/test001.json");
         Settings graphics_settings = settings.get_section("graphics", true);
         assert_true(graphics_settings.get<int>("width") == 1920);
         assert_true(graphics_settings.get<int>("height") == 1080);
         return true;
      })
      .test("Settings-002: Using default values for settings", [&]()->bool {
         Settings settings;
         Settings graphics_settings = settings.get_section("graphics");
         int width = graphics_settings.get<int>("width", 1920);
         int height = graphics_settings.get<int>("height", 1080);

         assert_true(width == 1920);
         assert_true(height == 1080);
         assert_true(graphics_settings.get<int>("width") == 1920);
         assert_true(graphics_settings.get<int>("height") == 1080);

         settings.set_section("graphics", graphics_settings);
         graphics_settings = settings.get_section("graphics");
         assert_true(graphics_settings.get<int>("width") == 1920);
         assert_true(graphics_settings.get<int>("height") == 1080);

         return true;
      })
      .test("Settings-003: Write a new settings value based on defaults", [&]()->bool {
         Settings settings;
         Settings graphics_settings = settings.get_section("graphics");

         int width = graphics_settings.get<int>("width", 1920);
         int height = graphics_settings.get<int>("height", 1080);

         assert_true(width == 1920);
         assert_true(height == 1080);
         assert_true(graphics_settings.get<int>("width") == 1920);
         assert_true(graphics_settings.get<int>("height") == 1080);

         settings.set_section("graphics", graphics_settings);
         settings.save_to_file("Settings-003.json.output");
         settings = Settings::load_from_file("Settings-003.json.output");

         graphics_settings = settings.get_section("graphics");
         assert_true(graphics_settings.get<int>("width") == 1920);
         assert_true(graphics_settings.get<int>("height") == 1080);

         return true;
      })
      .test("Settings-004: Load arrays from settings keys", [&]()->bool {
         Settings settings = Settings::load_from_file("settings/test004.json");

         vector<int> integers = settings.get_array<int>("numbers");
         vector<string> strings = settings.get_array<string>("strings");

         assert_true(lists_equal(integers, {1, 2, 3, 4, 5}));
         assert_true(lists_equal(strings, {"alpha", "bravo", "charlie", "delta", "eagle"}));
         assert_false(lists_equal(integers, {1, 2, 3, 4}));
         assert_false(lists_equal(strings, {"alpha", "bravo", "charlie", "delta"}));

         return true;
      })
      .test("Settings-005: Load and save arrays with defaults", [&]()->bool {
         Settings settings;

         vector<int> integers = settings.get_array<int>("numbers", {1, 2, 3, 4, 5});
         assert_true(lists_equal(integers, {1, 2, 3, 4, 5}));
         settings.save_to_file("Settings-005.json.output");
         settings = Settings::load_from_file("Settings-005.json.output");
         integers = settings.get_array<int>("numbers");
         assert_true(lists_equal(integers, {1, 2, 3, 4, 5}));

         return true;
      })
      .test("Settings-006: Heterogenous lists throw SettingsException", [&]()->bool {
         Settings settings = Settings::load_from_file("settings/test006.json");
         try {
            vector<int> integers = settings.get_array<int>("numbers");

         } catch (const SettingsException& e) {
            cerr << "Received expected SettingsException: "
                 << e.get_message()
                 << endl;
            return true;
         }

         return false;
      })
      .run();
}