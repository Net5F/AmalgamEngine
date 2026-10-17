#include "PlayerLocator.h"
#include "Cylinder.h"

namespace AM
{
namespace ChatServer
{

void PlayerLocator::setPosition(Sint64 accountID, const Vector3& position)
{
    playerPositions[accountID] = position;
}

void PlayerLocator::remove(Sint64 accountID)
{
    playerPositions.erase(accountID);
}

void PlayerLocator::clear()
{
    playerPositions.clear();
}

const Position* PlayerLocator::getPosition(Sint64 accountID) const
{
    auto positionIt{playerPositions.find(accountID)};
    if (positionIt == playerPositions.end()) {
        return nullptr;
    }

    return &(positionIt->second);
}

std::vector<Sint64>& PlayerLocator::getPlayers(const Cylinder& cylinder)
{
    returnVector.clear();
    for (const auto& [accountID, position] : playerPositions) {
        if (cylinder.intersects(position)) {
            returnVector.push_back(accountID);
        }
    }

    return returnVector;
}

} // End namespace ChatServer
} // End namespace AM
