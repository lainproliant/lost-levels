/* 
 * level: Data structures for representing a level made of layers.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, June 8 2015 (!)
 */
#pragma once
#include "resource_base.h"
#include "lain/string.h"
#include "tinyformat/tinyformat.h"

#include <vector>

namespace lost_levels {
   using namespace std;
   using namespace lain;

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

}
