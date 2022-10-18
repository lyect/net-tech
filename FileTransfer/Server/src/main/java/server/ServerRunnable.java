package server;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Logger;

public class ServerRunnable implements Runnable {

	private final int port;

	private ServerSocket serverSocket;
	private final ExecutorService clientThreadPool = Executors.newCachedThreadPool();
	private static Logger LOGGER;

	public ServerRunnable(int _port) {
		LOGGER = Logger.getLogger(ServerRunnable.class.getName());
		port = _port;
	}

	// Method for server starting.
	// If it completes normally, then server socket is opened.
	private void startServer() throws IOException {
		serverSocket = new ServerSocket(port);
		serverSocket.setSoTimeout(5000);
	}

	// Method for interrupting client handlers.
	private void interruptClientHandlers() {
		clientThreadPool.shutdownNow();
	}

	// Method for server closing.
	// If it completes normally, then server socket is closed
	//	and all client handlers are interrupted.
	private void closeServer() throws IOException {
		interruptClientHandlers();
		serverSocket.close();
	}

	@Override
	public void run() {

		// Starting server
		try {
			LOGGER.info("Starting server...");
			startServer();
			LOGGER.info("Server started successfully! Server is listening on port " + port + ".");
			System.out.println("Server started successfully! Server is listening on port " + port + ".");
		} catch (IOException ioe) {
			LOGGER.severe("Failed to start server on port " + port + ".");
			LOGGER.severe(ioe.getMessage());
			System.out.println("Failed to start server on port " + port + ".");
			ioe.printStackTrace();
			return;
		}

		while (true) {

			// Check if server is interrupted.
			// And if it is, then stop accepting loop.
			if (Thread.currentThread().isInterrupted()) {
				break;
			}

			try {
				// Accept incoming connection and start new thread with
				//	client handler for this connection
				Socket clientSocket = serverSocket.accept();
				clientThreadPool.execute(new ClientHandler(clientSocket));
			} catch (SocketTimeoutException e) { // Accept is timed out.
				// Don't care about timeout
			} catch (IOException e) { // Something except timeout happened. It is not good.
				LOGGER.severe(e.getMessage());
				break;
			}
		}

		if (!Thread.currentThread().isInterrupted()) {
			System.out.println("Server is down. Enter \"STOP\" command and check logs.");
		}

		// Closing server
		try {
			LOGGER.info("Closing server...");
			closeServer();
			LOGGER.info("Server closed successfully!");
		} catch (IOException ioe) {
			LOGGER.warning("Failed to close server properly.");
			LOGGER.warning(ioe.getMessage());
		}
	}

}
