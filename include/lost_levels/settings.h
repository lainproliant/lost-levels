/*
 * Settings: A picojson wrapper facilitating flat and
 *    nested settings structures with JSON.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Wednesday, Jan 14 2015
 */
#pragma once
#include "common.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;
   namespace pj = picojson;

   class SettingsException : public Exception {
   public:
      SettingsException(const string& msg) :
         Exception(msg) { }
      virtual ~SettingsException() { }
   };

   namespace settings_impl {
      template <class T>
      void set_value(pj::value& obj_value, const string& name, const T& value) {
         pj::object& obj = obj_value.get<pj::object>();
         pj::value json_value(value);
         obj[name] = json_value;
      }

      template <class T>
      T get_value(const pj::value& obj_value, const string& name) {
         if (! obj_value.contains(name)) {
            throw SettingsException(tfm::format(
               "Missing value for key '%s'.", name));
         }

         pj::value value = obj_value.get(name);
         if (! value.is<T>()) {
            throw SettingsException(tfm::format(
               "Unexpected value type for key '%s'.", name));
         }

         return value.get<T>();
      }

      template <class T>
      T get_value(pj::value& obj_value, const string& name, const T& default_value) {
         try {
            return get_value<T>(obj_value, name);

         } catch (const SettingsException& e) {
            set_value(obj_value, name, default_value);
            return default_value;
         }
      }

      template <class T>
      void set_array(pj::value& obj_value, const string& name, const vector<T>& vec) {
         pj::object& obj = obj_value.get<pj::object>();
         pj::array array;

         for (T val : vec) {
            array.push_back(pj::value(val));
         }

         obj[name] = pj::value(array);
      }

      template<class T>
      vector<T> get_array(const pj::value& obj_value, const string& name) {
         vector<T> vec;

         if (! obj_value.contains(name)) {
            throw SettingsException(tfm::format(
               "Missing array for key '%s'.", name));
         }

         const pj::value& array_value = obj_value.get(name);
         if (! array_value.is<pj::array>()) {
            throw SettingsException(tfm::format("Key '%s' does not refer to an array.", name));
         }

         const pj::array& array = array_value.get<pj::array>();
         for (pj::value val : array) {
            if (! val.is<T>()) {
               throw SettingsException(tfm::format(
                  "Unexpected heterogenous value type in array for key '%s'.", name));
            }

            vec.push_back(val.get<T>());
         }

         return vec;
      }

      template <class T>
      vector<T> get_array(pj::value& obj_value, const string& name, const vector<T>& default_array) {
         try {
            return get_array<T>(obj_value, name);

         } catch (const SettingsException& e) {
            cerr << e.get_message() << endl;
            set_array(name, default_array);
            return default_array;
         }
      }

      template <>
      int get_value<int>(const pj::value& obj_value, const string& name) {
         return (int)get_value<double>(obj_value, name);
      }

      template <>
      int get_value<int>(pj::value& obj_value, const string& name, const int& default_value) {
         return (int)(get_value<double>(obj_value, name,
                  (double)(default_value)));
      }

      template <>
      void set_array<int>(pj::value& obj_value, const string& name, const vector<int>& vec) {
         vector<double> db_vec;
         copy(vec.begin(), vec.end(), back_inserter(db_vec));
         set_array<double>(obj_value, name, db_vec);
      }

      template <>
      vector<int> get_array<int>(const pj::value& obj_value, const string& name) {
         vector<int> vec;
         vector<double> db_vec = get_array<double>(obj_value, name);
         copy(db_vec.begin(), db_vec.end(), back_inserter(vec));
         return vec;
      }

      template <>
      vector<int> get_array<int>(pj::value& obj_value, const string& name, const vector<int>& default_vec) {
         try {
            return get_array<int>(obj_value, name);

         } catch (const SettingsException& e) {
            set_array(obj_value, name, default_vec);
            return default_vec;
         }
      }
   }

   class Settings {
   public:
      Settings() : obj_value(make_shared<pj::value>(pj::object())) { }
      virtual ~Settings() { }

      static Settings load_from_file(const string& filename) {
         ifstream infile;
         shared_ptr<pj::value> obj_value = make_shared<pj::value>();
         infile.open(filename);
         infile >> (*obj_value);

         if (! obj_value->is<pj::object>()) {
            throw SettingsException(
               "JSON file does not contain an object.");
         }

         return Settings(obj_value);
      }

      void save_to_file(const string& filename, bool prettify = false) const {
         ofstream outfile;
         outfile.open(filename);
         outfile << obj_value->serialize(prettify);
      }

      template <class T>
      T get(const string& name) const {
         return settings_impl::get_value<T>(*const_pointer_cast<const pj::value>(obj_value), name);
      }

      template <class T>
      T get(const string& name, const T& default_value) {
         return settings_impl::get_value<T>(*obj_value, name, default_value);
      }

      template <class T>
      void set(string& name, const T& value) {
         settings_impl::set_value<T>(*const_pointer_cast<const pj::value>(obj_value), name, value);
      }

      template <class T>
      vector<T> get_array(const string& name) const {
         return settings_impl::get_array<T>(*obj_value, name);
      }

      template <class T>
      vector<T> get_array(const string& name, const vector<T>& default_vec) {
         return settings_impl::get_array<T>(*obj_value, name, default_vec);
      }

      template <class T>
      void set_array(const string& name, const vector<T>& vec) {
         settings_impl::set_array<T>(*obj_value, name, vec);
      }

      Settings get_section(const string& name, bool must_exist = false) const {
         if (! obj_value->contains(name)) {
            if (must_exist) {
               throw SettingsException(tfm::format("Missing section for key '%s'.", name));

            } else {
               return Settings();
            }
         }

         pj::value section_json = obj_value->get(name);
         if (! section_json.is<pj::object>()) {
            throw SettingsException(tfm::format("Key '%s' does not refer to a section.", name));
         }

         return Settings(make_shared<pj::value>(section_json));
      }

      void set_section(const string& name, const Settings& section) {
         pj::object& obj = obj_value->get<pj::object>();
         obj[name] = (*section.obj_value);
      }

   private:
      Settings(const shared_ptr<pj::value>& obj_value) :
         obj_value(obj_value) { }

      shared_ptr<pj::value> obj_value;
   };
}
