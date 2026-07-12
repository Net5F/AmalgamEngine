#pragma once

namespace AM
{
namespace AccountServer
{
/**
 * 
 */
class DataModel
{
public:
    DataModel();

private:
    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    //entt::sigh<void(const LibraryItemData& newActiveItem)>
    //    activeLibraryItemChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** The active library item has changed. */
    //entt::sink<entt::sigh<void(const LibraryItemData& newActiveItem)>>
    //    activeLibraryItemChanged;
};

} // namespace AccountServer
} // namespace AM
