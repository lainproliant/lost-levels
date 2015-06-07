/*
 * resource_base.h: Classes and enumerations for safe aggregation
 *    of objects which are managed as resources.
 *
 * Author: Lain Supe (lainproliant)
 * Date: June 7th 2015
 */
#pragma once

#include <string>
#include <map>

namespace lost_levels {
   using namespace std;

   class Resource {
   public:
      enum Type {
         AUDIO,
         IMAGE,
         ANIMATION,
         LEVEL_DATA,
         FONT,
         ANIMATED_FONT
      };

      Resource(Type rcType) : type(rcType) { }
      virtual ~Resource() { }

      Type get_type() const {
         return type;
      }

   private:
      Type type;
   };

   template<Resource::Type rcType>
   class ResourceImpl : public Resource {
   public:
      static const Resource::Type RC_TYPE = rcType;

      ResourceImpl() : Resource(rcType) { }
      virtual ~ResourceImpl() { }
   };

   inline const string& rc_type_to_string(const Resource::Type rcType) {
      static const map<Resource::Type, string> RC_TYPE_MAP = {
         {Resource::Type::AUDIO,          "Audio"},
         {Resource::Type::IMAGE,          "Image"},
         {Resource::Type::ANIMATION,      "Animation"},
         {Resource::Type::LEVEL_DATA,     "LevelData"},
         {Resource::Type::FONT,           "Font"},
         {Resource::Type::ANIMATED_FONT,  "AnimatedFont"}};

      static const string UNKNOWN = "Unknown";

      auto iter = RC_TYPE_MAP.find(rcType);

      return iter == RC_TYPE_MAP.end() ? UNKNOWN : iter->second;
   }
}
