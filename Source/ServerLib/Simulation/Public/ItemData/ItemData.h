#pragma once

#include "ItemDataBase.h"

namespace AM
{
namespace Server
{
/**
 * See ItemDataBase class comment.
 *
 * In addition, performs server-specific duties such as saving/loading item 
 * definitions.
 */
class ItemData : public ItemDataBase
{
public:
    /**
     * Calls ItemDataBase() constructor and loads our persisted items.
     */
    ItemData();
};

} // End namespace Server
} // End namespace AM
