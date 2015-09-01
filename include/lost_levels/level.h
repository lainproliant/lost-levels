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

#include "lost_levels/geometry.h"
#include "lost_levels/graphics.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   class LevelDataException : public Exception {
   public:
      using Exception::Exception;
   };

   class BlockData {
   public:
      BlockData(const string& name, int id) :
         name(name), id(id) { }
      
      string name;
      int id;
   };

   class BlockDataSet {
   public:
      BlockDataSet(const vector<BlockData>& blocks) {
         for (BlockData block : blocks) {
            blockMap[block.id] = block;
         }
      }

      inline const BlockData& get_block(int id) const {
         auto iter = blockMap.find(id);
         if (iter == blockMap.cend()) {
            throw LevelDataException(tfm::format("id not found in BlockDataSet: %d", id));
         }
         return iter->second;
      }
      
   private:
      map<int, BlockData> blockMap;
   };

   class BlockMap : public ResourceImpl<Resource::Type::BLOCK_MAP> {
   public:
      BlockMap(vector<BlockData> blocks, Size<int> levelSize) :
         blocks(blocks), levelSize(levelSize) {
         assert(levelSize.width * levelSize.height == blocks.size());
      }

      const BlockData& get_block(Point<int> pt) {
         assert(pt.x < levelSize.width);
         assert(pt.y < levelSize.height);

         size_t offset = pt.y * levelSize.width + pt.x;
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
