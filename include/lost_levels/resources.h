/*
 * resources: Resource sharing, loading, and management.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, Feb 2 2015
 */
#pragma once
#include "lain/settings.h"
#include "lost_levels/graphics.h"
#include "apathy/path.hpp"

namespace lost_levels {
   using namespace std;
   using namespace lain;
   namespace fs = apathy;

   class ResourceException : public Exception {
   public:
      using Exception::Exception;
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
                      shared_ptr<Renderer> renderer) :
         timer(timer), renderer(renderer) { }

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

      /**
       * Get an image resource with the specified name.
       *
       * @throws ResourceException if the resource of the specified name
       *    was not provided.
       */
      shared_ptr<Image> get_image(const string& name) const {
         auto iter = image_resources.find(name);
         if (iter != image_resources.end()) {
            return iter->second;

         } else {
            throw ResourceException(tfm::format(
               "Image resource '%s' was not provided.", name));
         }
      }

      /**
       * Get an animation resource with the specified name.
       *
       * @throws ResourceException if the resource of the specified name
       *    was not provided.
       */
      shared_ptr<Animation> get_animation(const string& name) const {
         auto iter = anim_resources.find(name);
         if (iter != anim_resources.end()) {
            return iter->second;

         } else {
            throw ResourceException(tfm::format(
               "Animation resource '%s' was not provided.", name));
         }
      }

      /**
       * Share an image resource of the specified name.  This can be used
       * to generate image resources at runtime and share them with
       * other parts of the game.
       *
       * @throws ResourceException if a resource of the specified name
       *    is already defined.
       */
      void share_image(const string& name, shared_ptr<Image> image) {
         if (image_resources.find(name) != image_resources.end()) {
            throw ResourceException(tfm::format(
               "Duplicate image resource name: '%s'", name));
         }

         image_resources.insert({name, image});
      }

      /**
       * Share an animation resource of the specified name.  This can be used
       * to generate animation resources at runtime and share them with
       * other parts of the game.
       *
       * @throws ResourceException if a resource of the specified name
       *    is already defined.
       */
      void share_animation(const string& name, shared_ptr<Animation> anim) {
        if (anim_resources.find(name) != anim_resources.end()) {
            throw ResourceException(tfm::format(
               "Duplicate animation resource name: '%s'", name));
        }

        anim_resources.insert({name, anim});
      }

      /**
       * Update all of the loaded animations.  This can be used to
       * synchronize animations across the game.
       *
       * NOTE:
       * - To allow an object to own its own animation, e.g. have it update
       *   out of sync with other objects using the same animation, create
       *   a copy of the animation via Animation::copy().
       */
      void update_animations() {
         for (auto& kv : anim_resources) {
            kv.second->update();
         }
      }

   protected:

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
         shared_ptr<Image> image = renderer->load_image(imagePath.string());
         share_image(object.get<string>("name"), image);
      }

      void load_animation_object_array(const vector<Settings>& obj_array) {
         for (Settings obj : obj_array) {
            load_animation_object(obj);
         }
      }

      void load_animation_object(const Settings& anim_entry) {
         Size<int> sz;
         string name = anim_entry.get<string>("name");

         if (anim_resources.find(name) != anim_resources.end()) {
            throw ResourceException(tfm::format(
               "Duplicate animation resource name: '%s'", name));
         }

         shared_ptr<Image> image = get_image(
               anim_entry.get<string>("image"));
         vector<Animation::Frame> frames = Animation::parse_frames(
            anim_entry.get_array<string>("frames"));
         sz.width = anim_entry.get<int>("width");
         sz.height = anim_entry.get<int>("height");

         bool looping = anim_entry.get_default<bool>("loop", false);

         shared_ptr<Animation> anim = Animation::create(
            image, sz, frames, timer, looping);

         anim_resources.insert({name, anim});
      }

   private:
      shared_ptr<const Timer<unsigned int>> timer;
      shared_ptr<Renderer> renderer;
      map<string, shared_ptr<Image>> image_resources;
      map<string, shared_ptr<Animation>> anim_resources;
   };
}
