#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include <SDL_stdinc.h>
#include <memory>
#include <string>

// Forward declaration
struct _TCPsocket;
typedef struct _TCPsocket* TCPsocket;

namespace AM
{
/**
 * Represents a single TCP socket.
 * Wraps SDLNet's TCPsocket in an RAII object interface.
 */
class TcpSocket
{
public:
    /**
     * Opens the socket as a listener.
     *
     * @param inPort  The port to listen on.
     */
    TcpSocket(Uint16 inPort);

    /**
     * Accepts the given SDLNet socket connection.
     *
     * @param inSdlSocket A connected socket.
     */
    TcpSocket(TCPsocket inSdlSocket);

    /**
     * Opens a socket connection to the given host.
     *
     * @param inIp  The IP to connect to.
     * @param inPort  The port to connect to.
     */
    TcpSocket(std::string inIp, Uint16 inPort);

    /**
     * Closes the socket.
     * Note: Will not remove the socket from any sets that it might belong to.
     */
    ~TcpSocket();

    // Not copyable.
    TcpSocket(const TcpSocket& otherSocket) = delete;
    TcpSocket& operator=(const TcpSocket& otherSocket) = delete;

    /**
     * Sends len bytes from the given dataBuffer over this socket.
     * @return The number of bytes sent. If the number returned is less than
     * len, then an error occurred, such as the client disconnecting.
     */
    int send(const void* dataBuffer, int len);

    /**
     * Receive data of exactly maxLen bytes from this socket, into the memory
     * pointed to by dataBuffer.
     *
     * Unless there is an error, or the connection is closed, the buffer will
     * read maxLen bytes. If you read more than is sent from the other end, then
     * it will wait until the full requested length is sent, or until the
     * connection is closed from the other end.
     *
     * Note: This function is not used for server (listener) sockets.
     *
     * @return The number of bytes received.
     *         If the number returned is <= 0, then an error occurred, or the
     * remote host has closed the connection.
     */
    int receive(void* dataBuffer, int maxLen);

    /**
     * Checks if a socket has been marked as active.
     *
     * Note: Only call this on a socket in a set, after calling checkSockets()
     * on that set.
     */
    bool isReady();

    /**
     * Accepts an incoming connection on this socket.
     *
     * Note: Only call this on a server (listener) socket.
     *
     * @return A valid TcpSocket unique pointer on success.
     *         nullptr is returned on errors, such as failure to create a
     * socket, failure to finish connecting, or if there is no waiting
     * connection.
     */
    std::unique_ptr<TcpSocket> accept();

    /**
     * Gets the address of the peer at the other side of the socket.
     *
     * Note: Only call this on a connected socket, not a listener.
     *
     * @return An address in the form "IP:port".
     */
    std::string getAddress();

    /**
     * Returns the transport library's underlying socket type.
     */
    TCPsocket getUnderlyingSocket() const;

private:
    TCPsocket socket;

    /** This socket's IP. Empty if this is a listener socket. */
    std::string ip;

    /** This socket's port. */
    Uint16 port;
};

} // End namespace AM

#endif /* End TCPSOCKET_H_ */
