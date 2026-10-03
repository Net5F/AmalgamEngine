#pragma once

#include "IconDataBase.h"

namespace AM
{
namespace WorldServer
{
/**
 * See IconDataBase class comment.
 *
 * The server doesn't currently do any parsing beyond what IconDataBase
 * performs, but this class exists to maintain a pattern consistent with
 * Client::IconData (which does do additional parsing).
 */
class IconData : public IconDataBase
{
public:
    /**
     * Calls IconDataBase() constructor.
     */
    IconData(const nlohmann::json& resourceDataJson);
};

} // End namespace WorldServer
} // End namespace AM
