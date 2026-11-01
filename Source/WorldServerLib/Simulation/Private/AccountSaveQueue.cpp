#include "AccountSaveQueue.h"
#include "Shared/AccountServiceEndpoint.h"
#include "Log.h"
#include "tracy/Tracy.hpp"
#include <utility>

namespace AM
{
namespace WorldServer
{
AccountSaveQueue::AccountSaveQueue(EventDispatcher& inNetworkEventDispatcher,
                                   AccountServiceEndpoint& inAccountEndpoint)
: networkEventDispatcher{inNetworkEventDispatcher}
, accountEndpoint{inAccountEndpoint}
, saveResultQueue{inNetworkEventDispatcher}
, pendingSaves{}
, inFlightCount{0}
{
}

void AccountSaveQueue::queueSave(Sint64 accountID, AccountData&& accountData)
{
    // If the data is too large, the account server can't save it. Keep
    // whatever data it already has.
    if (!(accountData.isWithinSizeLimits())) {
        LOG_ERROR("Account data exceeds size limit, skipping save. "
                  "AccountID: %lld, engine bytes: %zu, project bytes: %zu",
                  static_cast<long long>(accountID),
                  accountData.engineComponents.size(),
                  accountData.projectComponents.size());
        return;
    }

    // Replace any queued data. If a save is in flight, this will be sent
    // once it completes.
    // Note: We keep any retry delay, so a failing account server isn't
    //       spammed.
    PendingSave& pendingSave{pendingSaves[accountID]};
    pendingSave.accountData = std::move(accountData);
    pendingSave.needsSend = true;
}

const AccountData* AccountSaveQueue::getUnsavedData(Sint64 accountID) const
{
    auto saveIt{pendingSaves.find(accountID)};
    if (saveIt == pendingSaves.end()) {
        return nullptr;
    }

    return &(saveIt->second.accountData);
}

void AccountSaveQueue::processSaves()
{
    ZoneScoped;

    // Process any save results.
    SaveResult saveResult{};
    while (saveResultQueue.pop(saveResult)) {
        processSaveResult(saveResult);
    }

    // Send any queued saves, up to our in-flight limit.
    if (pendingSaves.empty() || (inFlightCount >= MAX_IN_FLIGHT_SAVES)) {
        return;
    }
    Clock::time_point currentTime{Clock::now()};
    for (auto& [accountID, pendingSave] : pendingSaves) {
        if (inFlightCount >= MAX_IN_FLIGHT_SAVES) {
            break;
        }

        if (pendingSave.needsSend && !(pendingSave.inFlight)
            && (!(pendingSave.retryTime)
                || (currentTime >= *(pendingSave.retryTime)))) {
            sendSave(accountID, pendingSave);
        }
    }
}

void AccountSaveQueue::processSaveResult(const SaveResult& saveResult)
{
    auto saveIt{pendingSaves.find(saveResult.accountID)};
    if (saveIt == pendingSaves.end()) {
        LOG_ERROR("Received save result for account with no pending save. "
                  "AccountID: %lld",
                  static_cast<long long>(saveResult.accountID));
        return;
    }

    PendingSave& pendingSave{saveIt->second};
    pendingSave.inFlight = false;
    inFlightCount--;

    switch (saveResult.result) {
        case SaveAccountDataResponse::Success: {
            // If no newer data was queued while this save was in flight,
            // we're done with this account.
            pendingSave.retryTime.reset();
            if (!(pendingSave.needsSend)) {
                pendingSaves.erase(saveIt);
            }
            break;
        }
        case SaveAccountDataResponse::InvalidAccount: {
            // Retrying won't help, so drop the data.
            LOG_ERROR("Failed to save account data: Invalid account. "
                      "AccountID: %lld",
                      static_cast<long long>(saveResult.accountID));
            pendingSaves.erase(saveIt);
            break;
        }
        default: {
            // Keep the data and retry after a delay.
            LOG_INFO("Failed to save account data, will retry. AccountID: "
                     "%lld",
                     static_cast<long long>(saveResult.accountID));
            pendingSave.needsSend = true;
            pendingSave.retryTime = Clock::now() + RETRY_DELAY;
            break;
        }
    }
}

void AccountSaveQueue::sendSave(Sint64 accountID, PendingSave& pendingSave)
{
    pendingSave.needsSend = false;
    pendingSave.inFlight = true;
    inFlightCount++;

    // Note: We send a copy, since we need to keep the data until the save is
    //       confirmed.
    // Note: The callback is called on the network IO thread, so we push the
    //       result to be processed on the next tick.
    accountEndpoint.saveAccountData(
        accountID, pendingSave.accountData,
        [this, accountID](const SaveAccountDataResponse& response) {
            networkEventDispatcher.emplace<SaveResult>(accountID,
                                                       response.result);
        });
}

} // namespace WorldServer
} // namespace AM
