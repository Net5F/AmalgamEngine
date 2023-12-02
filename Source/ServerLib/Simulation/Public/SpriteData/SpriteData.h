#pragma once

#include "SpriteDataBase.h"

namespace AM
{
namespace Server
{
/**
 * See SpriteDataBase class comment.
 *
 * The server doesn't currently do any parsing beyond what SpriteDataBase
 * performs, but this class exists to maintain a pattern consistent with
 * Client::SpriteData (which does do additional parsing).
 */
class SpriteData : public SpriteDataBase
{
public:
    /**
     * Calls SpriteDataBase() constructor.
     */
    SpriteData(const nlohmann::json& resourceDataJson);
};

} // End namespace Server
} // End namespace AM
