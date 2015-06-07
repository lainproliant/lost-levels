/*
 * resources: Resource sharing, loading, and management.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, Feb 2 2015
 */
#pragma once
#include "lain/settings.h"
#include "lost_levels/graphics.h"
#include "lost_levels/resource_base.h"
#include "apathy/path.hpp"

namespace lost_levels {
   using namespace std;
   using namespace lain;
   namespace fs = apathy;

   inline string rc_format(string name, Resource::Type rcType) {
      return tfm::format("%s:%s", name, rc_type_to_string(rcType));
   }

   class ResourceException : public Exception {
   public:
      using Exception::Exception;
   };

   class BlockData {
   public:
      BlockData(int blocksetId, int blockId) :
         set(blocksetId), id(blockId) { }

      static BlockData parse(const string& blockDataExpr) {
         vector<string> tuple;
         str::split(tuple, blockDataExpr, ",");

         if (tuple.size() != 2) {
            throw ResourceException(tfm::format(
                     "Invalid BlockData expression: '%s'", blockDataExpr));
         }

         try {
            return BlockData(stoi(tuple[0]), stoi(tuple[1]));

         } catch (const invalid_argument& e) {
            throw ResourceException(tfm::format(
                     "Invalid BlockData expression (non-numeric): '%s'", blockDataExpr));
         }
      }

      string to_string() const {
         return tfm::format("%d,%d", set, id);
      }

      int set;
      int id;
   };

   class LevelData : public ResourceImpl<Resource::Type::LEVEL_DATA> {
   public:
      LevelData(vector<BlockData> blocks, Size<int> levelSize) :
         blocks(blocks), levelSize(levelSize) {

         assert(levelSize.width * levelSize.height == blocks.size());
      }

      const BlockData& get_block(Point<int> pt) {
         size_t offset = pt.y * levelSize.width + pt.x;
         assert(offset >= blocks.size());
         return blocks[offset];
      }

      const Size<int>& get_level_size() const {
         return levelSize;
      }

   private:
      vector<BlockData> blocks;
      Size<int> levelSize;
   };

   /**
    * An object for managing and sharing resources in a game.
    *
    * USAGE:
    * - Create an instance of the ResourceManager and share it
    *   appropriately across components of your game.  Use it to manage
    *   the lifespan and availability of shared resources such as images,
    *   animations, and sounds.  You can create one for the entire game,
    *   or load one for each level or screen.
    * - Load the resource manager by pointing it to one or more resource
    *   files.
    * - Use the resources, referring to them with names defined in the
    *   resource file(s).
    */
   class ResourceManager {
   public:
      ResourceManager(shared_ptr<Timer<unsigned int>> timer,
                      shared_ptr<ImageLoader> imageLoader) :
         timer(timer), imageLoader(imageLoader) { }

      /**
       * Load resources defined in the resource file at the given location.
       *
       * This method can throw ResourceException if something goes wrong
       * while loading the resource file.
       */
      void load_file(const string& filename) {
         try {
            Settings settings = Settings::load_from_file(filename);
            fs::Path base = fs::Path(filename).parent();

            if (settings.contains("images")) {
               load_image_object_array(base, settings.get_object_array("images"));
            }

            if (settings.contains("animations")) {
               load_animation_object_array(settings.get_object_array("animations"));
            }

         } catch (const exception& e) {
            throw ResourceException(tfm::format("Failed to load resource file '%s': %s",
               filename, e.what()));
         }
      }

      template<class T>
      shared_ptr<T> get(const string& name) const {
         const string rc_name = rc_format(name, T::RC_TYPE);
         if (resource_map.find(rc_name) == resource_map.end()) {
            throw ResourceException(tfm::format("No resource found with name '%s'.", name));
         }

         return dynamic_pointer_cast<T>(resource_map.at(rc_name));
      }

      void put(const string& name, shared_ptr<Resource> resource) {
         if (resource_map.find(name) != resource_map.end()) {
            throw ResourceException(tfm::format("A resource is already defined with name '%s'.", name));
         }

         resource_map[rc_format(name, resource->get_type())] = resource;
      }

   protected:
      void load_include(fs::Path& base, const string& filename) {
         fs::Path includePath = base.relative(filename);
         if (! includePath.exists()) {
            throw ResourceException(tfm::format("Included file '%s' does not exist.", filename));
         }

         load_file(includePath.string());
      }

      void load_image_object_array(const fs::Path& base,
                                   const vector<Settings>& obj_array) {

         for (Settings obj : obj_array) {
            load_image_object(base, obj);
         }
      }

      void load_image_object(const fs::Path& baseIn,
                             const Settings& object) {
         fs::Path base = baseIn;
         fs::Path imagePath = base.relative(object.get<string>("file"));
         shared_ptr<Image> image = imageLoader->load_image(imagePath.string());
         put(object.get<string>("name"), image);
      }

      void load_animation_object_array(const vector<Settings>& obj_array) {
         for (Settings obj : obj_array) {
            load_animation_object(obj);
         }
      }

      void load_animation_object(const Settings& anim_entry) {
         Size<int> sz;
         string name = anim_entry.get<string>("name");

         shared_ptr<Image> image = get<Image>(anim_entry.get<string>("image"));
         vector<Animation::Frame> frames = Animation::parse_frames(
            anim_entry.get_array<string>("frames"));
         sz.width = anim_entry.get<int>("width");
         sz.height = anim_entry.get<int>("height");

         bool looping = anim_entry.get_default<bool>("loop", false);

         shared_ptr<Animation> anim = Animation::create(
            image, sz, frames, timer, looping);

         put(name, anim);
      }

      void load_level_object_array(const vector<Settings>& obj_array) {
         for (Settings obj : obj_array) {
            load_level_object(obj);
         }
      }

      void load_level_object(const Settings& obj) {
         vector<BlockData> blocks;
         Size<int> levelSize;
         string name = obj.get<string>("name");

         levelSize = Size<int>(obj.get<int>("width"), obj.get<int>("height"));
         for (string blockExpr : obj.get_array<string>("blocks")) {
            blocks.push_back(BlockData::parse(blockExpr));
         }

         put(name, make_shared<LevelData>(blocks, levelSize));
      }

   private:
      shared_ptr<const Timer<unsigned int>> timer;
      shared_ptr<ImageLoader> imageLoader;

      map<string, shared_ptr<Resource>> resource_map;
   };
}
