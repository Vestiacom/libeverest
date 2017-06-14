#include "connection.hpp"
#include "logger.hpp"
#include "receiver.hpp"
#include "sender.hpp"

#include <vector>
#include <map>
#include <ev++.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace everest {
namespace internals {


Connection::Connection(int fd,
                       struct ev_loop* evLoop,
                       const InputDataCallback& inputDataCallback)
	: mFD(fd)
{
	if (!evLoop) {
		THROW("ev_loop is null");
	}

	if (mFD < 0) {
		THROW("bad fd");
	}

	mReceiver = std::unique_ptr<Receiver>(new Receiver(fd, evLoop, *this, inputDataCallback));
	mSender = std::unique_ptr<Sender>(new Sender(fd, evLoop));
}

Connection::~Connection()
{
	shutdown();
}

void Connection::start()
{
	mReceiver->start();
	mSender->start();
}

void Connection::stop()
{
	mReceiver->stop();
	mSender->stop();
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
	mReceiver->shutdown();
	mSender->shutdown();

	if (-1 == ::close(mFD)) {
		LOGW("close() failed with: " << std::strerror(errno));
	}

	mFD = -1;
}

bool Connection::isClosed()
{
	return mReceiver->isClosed() || mSender->isClosed() || mFD == -1;
}

void Connection::send(const std::shared_ptr<Response>& response)
{
	mSender->send(response);
}

} // namespace internals
} // namespace everest
