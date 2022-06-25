#pragma once

#include "OSEventHandler.h"
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
    /**
     * Calls the project's UI tick(), if present.
     */
    void tick(double timestepS);

    /**
     * Calls the project's UI render(), if present.
     */
    void render(const Camera& camera);

    /**
     * Passes the given event to the project's UI handler, if present.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<IUserInterfaceExtension> inExtension);

private:
    /** If non-nullptr, contains the project's UI extension functions.
        Allows the project to provide UI code and have it be called at the 
        appropriate time. */
    std::unique_ptr<IUserInterfaceExtension> extension;
};

} // End namespace Client
} // End namespace AM
