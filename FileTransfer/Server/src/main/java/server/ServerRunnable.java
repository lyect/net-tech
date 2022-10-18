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
	private final ExecutorService clientThreadPool = Executors.newCachedThreadPool();
	private static Logger LOGGER;

	public ServerRunnable(int _port) {
		LOGGER = Logger.getLogger(ServerRunnable.class.getName());
		port = _port;
	}

	// Method for interrupting client handlers.
	private void interruptClientHandlers() {
		clientThreadPool.shutdownNow();
	}

	@Override
	public void run() {

		// Starting server
		try (ServerSocket serverSocket = new ServerSocket(port)){
			// Set socket timeout
			serverSocket.setSoTimeout(5000);

			// Log about successful server start
			LOGGER.info("Server started successfully! Server is listening on port " + port + ".");
			System.out.println("Server started successfully! Server is listening on port " + port + ".");

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
				}
			}

			interruptClientHandlers();

			LOGGER.info("Server is closed.");

		} catch (IOException ioe) {
			LOGGER.severe("Failed to start server on port " + port + ".");
			LOGGER.severe(ioe.getMessage());
			System.out.println("Failed to start server on port " + port + ".");
			ioe.printStackTrace();
		}
	}
}