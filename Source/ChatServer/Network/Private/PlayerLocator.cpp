#include "PlayerLocator.h"

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

} // End namespace ChatServer
} // End namespace AM
