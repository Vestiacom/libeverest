<p align="center">
  <img src="https://github.com/Vestiacom/libeverest/blob/master/doc/everest.png?raw=true" />
</p>

# Everest
LibEverest is a library for embedding a HTTP server inside a C++ application. It uses [libev](http://software.schmorp.de/pkg/libev.html) and [http-parser](https://github.com/nodejs/http-parser). 

# Build
```sh
mkdir build && cd build
cmake ..
make
sudo make install
```

# Example usage
```cpp

#include <ev++.h>
#include <everest/everest.hpp>

int setup() {
	// Server configuration
	everest::Config serverConfig("localhost:5000");
	serverConfig.cleanupPeriodSec = 5; // seconds
	serverConfig.maxConnections = 1000; // max concurrent connections

	struct ev_loop* loop = EV_DEFAULT;

	everest::Server server(serverConfig, loop);
	server.endpoint("/v1/status", [&](const std::shared_ptr<everest::Request>& r) {
		auto response = r->createResponse();
		response->setHeader("A", "B");
		response->setStatus(200);
		response->appendBody("{\"status\":\"OK\"}");
		response->send();
	});

	server.endpoint("/v1/stop", [&](const std::shared_ptr<everest::Request>& r) {
		server.stop();
		ev_break(loop, EVBREAK_ALL);
	});

	// Starts accepting connections
	server.start();

	// Runs till ev_break is called
	ev_run(loop, 0);
}
```
