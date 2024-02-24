#pragma once

#include "GraphicDataBase.h"

namespace AM
{
namespace Server
{
/**
 * See GraphicDataBase class comment.
 *
 * The server doesn't currently do any parsing beyond what GraphicDataBase
 * performs, but this class exists to maintain a pattern consistent with
 * Client::GraphicData (which does do additional parsing).
 */
class GraphicData : public GraphicDataBase
{
public:
    /**
     * Calls GraphicDataBase() constructor.
     */
    GraphicData(const nlohmann::json& resourceDataJson);
};

} // End namespace Server
} // End namespace AM
