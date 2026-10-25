#include "Application.h"
#include "Timer.h"
#include "Config.h"
#include "SharedConfig.h"
#include "UserConfig.h"
#include "Paths.h"
#include "SDLHelpers.h"
#include "Log.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <chrono>
#include <memory>

namespace AM
{
namespace Client
{
Application::Application()
: sdl{SDL_INIT_VIDEO}
, userConfigInitializer{}
, sdlWindow{Config::WINDOW_TITLE, UserConfig::get().getWindowSize().w,
            UserConfig::get().getWindowSize().h, 0}
, sdlRenderer{sdlWindow.get(), nullptr}
, simEventDispatcher{}
, uiEventDispatcher{}
, networkEventDispatcher{}
, assetCache{sdlRenderer.get()}
, resourceData{}
, graphicData{resourceData.get(), assetCache}
, iconData{resourceData.get()}
, itemData{}
, castableData{graphicData}
, messageProcessorContext{networkEventDispatcher}
, network{messageProcessorContext}
, accountSession{network, networkEventDispatcher}
, chatSession{network, accountSession, networkEventDispatcher}
, simulationContext{simulation,
                    network,
                    simEventDispatcher,
                    uiEventDispatcher,
                    networkEventDispatcher,
                    graphicData,
                    iconData,
                    itemData,
                    castableData}
, simulation{simulationContext}
, uiContext{simulation,
            userInterface.getWorldObjectLocator(),
            network,
            accountSession,
            chatSession,
            uiEventDispatcher,
            simEventDispatcher,
            networkEventDispatcher,
            sdlRenderer.get(),
            graphicData,
            iconData,
            itemData}
, userInterface{uiContext}
, rendererContext{sdlRenderer.get(), simulation.getWorld(), userInterface,
                  [&]() { return simCaller.getProgress(); }, graphicData}
, renderer{rendererContext}
, networkCaller{std::bind_front(&WorldClientEndpoint::tick,
                                &network.worldEndpoint),
                SharedConfig::CLIENT_NETWORK_TICK_TIMESTEP_S, "Network", true}
, uiCaller{std::bind_front(&UserInterface::tick, &userInterface),
           Config::UI_TICK_TIMESTEP_S, "UserInterface", true}
, simCaller{std::bind_front(&Simulation::tick, &simulation),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false}
, rendererCaller{std::bind_front(&Renderer::render, &renderer),
                 UserConfig::get().getFrameTimestepS(), "Renderer", true}
, messageProcessorExtension{}
, rendererExtension{}
, simulationExtension{}
, userInterfaceExtension{}
, osLatenesses{}
, callerLatenesses{}
, iterationCount{0}
, idleIterationCount{0}
, lastTimingReportTime{}
, eventHandlers{this, &renderer, &userInterface, &simulation}
, exitRequested{false}
{
    // Initialize the global timer.
    Timer::getGlobalTime();

    // Set fullscreen mode.
    unsigned int fullscreenMode{UserConfig::get().getFullscreenMode()};
    switch (fullscreenMode) {
        case 0: {
            SDLHelpers::setWindowFullscreen(sdlWindow.get(), 0);
            break;
        }
        case 1: {
            SDLHelpers::setWindowFullscreen(sdlWindow.get(),
                                            SDL_WINDOW_FULLSCREEN);
            break;
        }
        // Note: We removed real fullscreen because it was behaving weirdly and
        //       causing network timeouts, and having just windowed and
        //       borderless seems fine.
        default: {
            LOG_FATAL("Invalid fullscreen value: %d", fullscreenMode);
            break;
        }
    }

    // Enable delay reporting.
    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);

    // Set up our event filter.
    SDL_SetEventFilter(&Application::filterEvents, this);
}

void Application::start()
{
    // If any of the extensions aren't registered, exit early.
    if (!messageProcessorExtension || !rendererExtension || !simulationExtension
        || !userInterfaceExtension) {
        LOG_FATAL("All extensions must be registered before calling start()");
    }

    // Start the network thread.
    network.start();

    // Prime the timers so they don't start at 0.
    simCaller.initTimer();
    uiCaller.initTimer();
    networkCaller.initTimer();
    rendererCaller.initTimer();
    lastTimingReportTime = std::chrono::steady_clock::now();

    while (!exitRequested) {
        recordIteration();
        runIteration();
        waitForNextCall();
    }
}

bool Application::handleOSEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_EVENT_QUIT: {
            exitRequested = true;
            return true;
        }
    }

    return false;
}

void Application::dispatchOSEvents()
{
    // Dispatch all waiting SDL events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Pass the event to each handler in order, stopping if it returns as
        // handled.
        for (OSEventHandler* handler : eventHandlers) {
            if (handler->handleOSEvent(event)) {
                break;
            }
        }
    }
}

bool Application::enoughTimeTillNextCall(double minimumTime)
{
    if ((simCaller.getTimeTillNextCall() > minimumTime)
        && (networkCaller.getTimeTillNextCall() > minimumTime)
        && (rendererCaller.getTimeTillNextCall() > minimumTime)) {
        return true;
    }
    else {
        return false;
    }
}

bool Application::filterEvents(void*, SDL_Event*)
{
    // Application* app{static_cast<Application*>(userData)};

    // Currently no events that we care to filter.
    //
    // switch (event->type) {
    // }

    return true;
}

void Application::runIteration()
{
    // Let the simulation process an iteration if it needs to.
    simCaller.update();

    // Process account and chat events before the UI observes session state.
    // Note: Chat must be after account, since it follows the login state.
    accountSession.tick();
    chatSession.tick();

    // Let the UI widgets tick if they need to.
    uiCaller.update();

    // Send a heartbeat if necessary.
    networkCaller.update();

    // Let the renderer render if it needs to.
    rendererCaller.update();

    // If we have enough time, dispatch events.
    if (enoughTimeTillNextCall(DISPATCH_MINIMUM_TIME_S)) {
        dispatchOSEvents();
    }
}

void Application::waitForNextCall()
{
    // If we have enough time, sleep until shortly before the next caller is
    // due.
    double timeTillNextCall{getMinTimeTillNextCall()};
    if (timeTillNextCall > SPIN_TIME_S) {
        double sleepTimeS{timeTillNextCall - SPIN_TIME_S};
        auto sleepStartTime{std::chrono::steady_clock::now()};
        SDL_DelayNS(static_cast<Uint64>(sleepTimeS * SDL_NS_PER_SECOND));
        recordOsLateness(std::chrono::duration<double>(
                             std::chrono::steady_clock::now() - sleepStartTime)
                             .count()
                         - sleepTimeS);
    }

    // Spin until the next caller is due.
    while (getMinTimeTillNextCall() > 0) {
        SDL_CPUPauseInstruction();
    }
}

double Application::getMinTimeTillNextCall()
{
    return std::min({simCaller.getTimeTillNextCall(),
                     uiCaller.getTimeTillNextCall(),
                     networkCaller.getTimeTillNextCall(),
                     rendererCaller.getTimeTillNextCall()});
}

void Application::recordOsLateness(double latenessS)
{
    osLatenesses.push_back(latenessS);
}

void Application::recordIteration()
{
    iterationCount++;

    // If a caller is due, record how overdue it is. Otherwise, this iteration
    // won't call anything.
    double callerLatenessS{-getMinTimeTillNextCall()};
    if (callerLatenessS >= 0) {
        callerLatenesses.push_back(callerLatenessS);
    }
    else {
        idleIterationCount++;
    }

    auto now{std::chrono::steady_clock::now()};
    double elapsedS{
        std::chrono::duration<double>(now - lastTimingReportTime).count()};
    if (elapsedS >= TIMING_REPORT_INTERVAL_S) {
        logTimingStats(elapsedS);
        lastTimingReportTime = now;
    }
}

void Application::logTimingStats(double elapsedS)
{
    std::sort(osLatenesses.begin(), osLatenesses.end());
    std::sort(callerLatenesses.begin(), callerLatenesses.end());

    LOG_INFO("Main loop: %u iterations (%.1f/s), %u idle. %zu OS waits, "
             "lateness (ms): p50 %.3f, p90 %.3f, p99 %.3f, max %.3f. "
             "Caller lateness (ms): p50 %.3f, p90 %.3f, p99 %.3f, max %.3f.",
             iterationCount, (iterationCount / elapsedS), idleIterationCount,
             osLatenesses.size(), getPercentile(osLatenesses, .5) * 1000,
             getPercentile(osLatenesses, .9) * 1000,
             getPercentile(osLatenesses, .99) * 1000,
             getPercentile(osLatenesses, 1) * 1000,
             getPercentile(callerLatenesses, .5) * 1000,
             getPercentile(callerLatenesses, .9) * 1000,
             getPercentile(callerLatenesses, .99) * 1000,
             getPercentile(callerLatenesses, 1) * 1000);

    iterationCount = 0;
    idleIterationCount = 0;
    osLatenesses.clear();
    callerLatenesses.clear();
}

double Application::getPercentile(const std::vector<double>& sortedValues,
                                  double percentile)
{
    if (sortedValues.empty()) {
        return 0;
    }

    std::size_t index{static_cast<std::size_t>(
        percentile * static_cast<double>(sortedValues.size() - 1))};
    return sortedValues[index];
}

} // End namespace Client
} // End namespace AM
