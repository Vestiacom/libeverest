#include "connection.hpp"

#include <vector>
#include <map>
#include <ev++.h>
#include <unistd.h>

// TODO: Reomove
#include <iostream>
using namespace std;


namespace everest {
namespace internals {


Connection::Connection(int fd,
                       struct ev_loop* evLoop,
                       const InputDataCallback& inputDataCallback)
	: mReceiver(fd, evLoop, *this, inputDataCallback),
	  mSender(fd, evLoop),
	  mFD(fd)
{
	if (!evLoop) {
		throw std::runtime_error("ev_loop is null");
	}

	if (mFD < 0) {
		throw std::runtime_error("bad fd");
	}
}

Connection::~Connection()
{
	shutdown();
}

void Connection::start()
{
	mReceiver.start();
	mSender.start();
}

void Connection::stop()
{
	mReceiver.stop();
	mSender.stop();
}

int Connection::getFD()
{
	return mFD;
}

void Connection::shutdown()
{
	if (mFD < 0) {
		return;
	}

	// No communication with the socket is possible after shutdown
	mReceiver.shutdown();
	mSender.shutdown();

	::close(mFD);
	mFD = -1;
}

bool Connection::isClosed()
{
	return mReceiver.isClosed() || mSender.isClosed() || mFD == -1;
}

void Connection::send(const std::shared_ptr<Response>& response)
{
	mSender.send(response);
}

} // namespace internals
} // namespace everest
