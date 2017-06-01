#include "connection.hpp"

#include <vector>
#include <ev++.h>
#include <unistd.h>

namespace everest {
namespace internals {


Connection::Connection(int fd,
                       struct ev_loop* evLoop,
                       const InputDataCallback& inputDataCallback)
	: mInputWatcher(evLoop),
	  mOutputWatcher(evLoop),
	  mFD(fd),
	  mInputDataCallback(inputDataCallback)
{
	if (!evLoop) {
		throw std::runtime_error("ev_loop is null");
	}

	if (mFD < 0) {
		throw std::runtime_error("bad fd");
	}

	mInputWatcher.set<Connection, &Connection::onInput>(this);
	mOutputWatcher.set<Connection, &Connection::onOutput>(this);
}


Connection::~Connection()
{
	mInputWatcher.stop();
	mOutputWatcher.stop();

	::close(mFD);
}

void Connection::start()
{
	mInputWatcher.start(mFD, ev::READ);
	mOutputWatcher.start(mFD, ev::WRITE);
}

void Connection::stop()
{
	mInputWatcher.stop();
	mOutputWatcher.stop();
}

void Connection::onInput(ev::io& w, int revents)
{
	if (EV_ERROR & revents) {
		// Unspecified error
		return;
	}

	(void) w;

	if (mInputDataCallback) {
		mInputDataCallback();
	}

}

void Connection::onOutput(ev::io& w, int revents)
{
	if (EV_ERROR & revents) {
		// Unspecified error
		return;
	}

	(void) w;
	// TODO: Implement async data sending
}

void Connection::send()
{

}


} // namespace internals
} // namespace everest

