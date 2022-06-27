#pragma once

#include "OSEventHandler.h"
#include "QueuedEvents.h"
#include <memory>

namespace AM
{
struct Camera;

namespace Client
{

class IUserInterfaceExtension;

/**
 * Passthrough class for the project's UI.
 */
class UserInterface : public OSEventHandler
{
public:
    UserInterface();

    /**
     * Calls the project's UI tick(), if present.
     *
     * @param tick  The time since this function was last called.
     */
    void tick(double timestepS);

    /**
     * Calls the project's UI render(), if present.
     *
     * @param camera  The camera to calculate screen position with.
     */
    void render(const Camera& camera);

    /**
     * Passes the given event to the project's UI handler, if present.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * Returns the Network event dispatcher. All messages that we receive
     * from the server are pushed into this dispatcher.
     */
    EventDispatcher& getEventDispatcher();

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<IUserInterfaceExtension> inExtension);

private:
    /** Used to dispatch events from the UI to the simulation. */
    EventDispatcher eventDispatcher;

    /** If non-nullptr, contains the project's UI extension functions.
        Allows the project to provide UI code and have it be called at the
        appropriate time. */
    std::unique_ptr<IUserInterfaceExtension> extension;
};

} // End namespace Client
} // End namespace AM
