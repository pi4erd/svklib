#include "window.hpp"
#include "log.hpp"
#include <stdexcept>

class App : public Window {
public:
    App() : Window("Window Example", {}) {
        initVulkan({}, true);
    }

    void loop(double delta) {

    }
};

int main(void) {
    logging::set_log_level(LOGLEVEL_DEBUG);

    App *app;
    try {
        app = new App();
    } catch(std::runtime_error &error) {
        LOG_ERROR("Error occured while initializing application: {}", error.what());
        return 1;
    }

    while(!app->shouldClose()) {
        app->pollEvents();

        app->loop(0.0);
    }

    delete app;
}