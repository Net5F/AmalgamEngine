#pragma once

#include "AccountData.h"
#include "SaveAccountDataResponse.h"
#include "QueuedEvents.h"
#include <SDL3/SDL_stdinc.h>
#include <chrono>
#include <cstddef>
#include <optional>
#include <unordered_map>

namespace AM
{
class AccountServiceEndpoint;

namespace WorldServer
{

/**
 * Saves client entities' AccountData (their ClientPersistence::Account
 * components) to the account server.
 *
 * Saves are queued by SaveHelper, both periodically and when a client entity
 * is removed. Each account's latest data is held here until the account
 * server confirms that it was saved. If the account server can't be reached
 * or the save fails, we keep the data and retry after a delay, so it isn't
 * lost (even if the entity has already been removed).
 *
 * Only 1 save per account is in flight at a time. If new data is queued while
 * a save is in flight, it's sent once that save completes. This keeps an
 * account's saves in order, since the account server may process requests
 * out of order.
 *
 * When the server shuts down, flush() is used to wait for the remaining
 * saves. If they can't be completed before its timeout, their data is lost.
 */
class AccountSaveQueue
{
public:
    AccountSaveQueue(EventDispatcher& inNetworkEventDispatcher,
                     AccountServiceEndpoint& inAccountEndpoint);

    /**
     * Queues the given account's data to be saved to the account server.
     * Replaces any data that's queued but not yet sent.
     *
     * If the data is too large to be saved, logs an error and discards it.
     */
    void queueSave(Sint64 accountID, AccountData&& accountData);

    /**
     * Returns the given account's latest queued data, if the account server
     * hasn't confirmed saving it yet.
     *
     * When an account logs in, this data is newer than what the account
     * server sent us, so it should be used instead.
     */
    const AccountData* getUnsavedData(Sint64 accountID) const;

    /**
     * Processes save results, and sends any queued saves (including retries
     * that are due).
     *
     * Call once per tick.
     */
    void processSaves();

    /**
     * Blocks while processing saves, until every queued save is confirmed or
     * the given timeout passes (failed saves are retried as usual).
     *
     * Call this once when the server is shutting down, after the last saves
     * are queued.
     *
     * @return The number of accounts whose data couldn't be saved.
     */
    std::size_t flush(std::chrono::seconds timeout);

private:
    using Clock = std::chrono::steady_clock;

    /** How long to wait before retrying a failed save. */
    static constexpr std::chrono::seconds RETRY_DELAY{5};

    /** The max number of saves that we'll have in flight at once. Limits how
        much data we hand to the network at once. */
    static constexpr std::size_t MAX_IN_FLIGHT_SAVES{32};

    /** How often flush() checks for save results. */
    static constexpr std::chrono::milliseconds FLUSH_POLL_INTERVAL{10};

    struct PendingSave {
        /** The account's latest data. */
        AccountData accountData{};

        /** If true, accountData hasn't been sent yet. */
        bool needsSend{true};

        /** If true, a save request is awaiting a response. */
        bool inFlight{false};

        /** If set, we won't send until this time (used after a failure). */
        std::optional<Clock::time_point> retryTime{};
    };

    /** A save result, pushed from the network IO thread. */
    struct SaveResult {
        Sint64 accountID{0};
        SaveAccountDataResponse::Result result{
            SaveAccountDataResponse::InternalError};
    };

    /**
     * Updates the given account's pending save with the given result.
     */
    void processSaveResult(const SaveResult& saveResult);

    /**
     * Sends the given account's latest data.
     */
    void sendSave(Sint64 accountID, PendingSave& pendingSave);

    /** Used to push save results from the network IO thread. */
    EventDispatcher& networkEventDispatcher;

    /** Used to send saves to the account server. */
    AccountServiceEndpoint& accountEndpoint;

    /** Receives save results from the network IO thread. */
    EventQueue<SaveResult> saveResultQueue;

    /** Maps account IDs -> their data that hasn't been confirmed saved. */
    std::unordered_map<Sint64, PendingSave> pendingSaves;

    /** The number of saves that are awaiting a response. */
    std::size_t inFlightCount;
};

} // namespace WorldServer
} // namespace AM
