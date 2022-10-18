package server;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;

public class ClientHandler implements Runnable {

	private final Socket passedSocket;
	private final String clientAddress;

	private File receivedFile;
	private static Logger LOGGER;

	// Time interval for calculation speed
	private final int PERIOD = 3; // seconds

	private String makeMessage(String message) {
		if (message == null) {
			return "[" + clientAddress + "]: Null message.";
		}
		return "[" + clientAddress + "]: " + message;
	}

	public ClientHandler(Socket _socket) {

		LOGGER = Logger.getLogger(ServerRunnable.class.getName());

		passedSocket = _socket;

		// Get client address without port and forward slash
		clientAddress = (((InetSocketAddress)passedSocket.getRemoteSocketAddress()).getAddress()).toString().replace("/","");
	}

	// This method reads integer from the socket
	private int readIntFromStream(DataInputStream stream) throws Exception {
		try {
			return stream.readInt();
		} catch (EOFException eofe) {
			LOGGER.severe(makeMessage("Failed to read integer, met EOF."));
			throw new Exception(); // Java-style bad results handling
		} catch (IOException ioe) {
			LOGGER.severe(makeMessage("IOException while reading integer."));
			LOGGER.severe(makeMessage(ioe.getMessage()));
			throw new Exception(); // Java-style bad results handling
		}
	}

	// This method reads UTF-8 string from the socket
	private String readStringFromStream(DataInputStream stream) throws Exception {
		int byteMessageLength = readIntFromStream(stream);

		if (byteMessageLength == 0) {
			LOGGER.severe(makeMessage("Failed to read string."));
			throw new Exception();
		}

		try {
			byte[] byteMessage = new byte[byteMessageLength];
			stream.readFully(byteMessage);
			return new String(byteMessage, StandardCharsets.UTF_8);
		} catch (EOFException eofe) {
			LOGGER.severe(makeMessage("Failed to read string, met EOF."));
			throw new Exception(); // Java-style bad results handling
		} catch (IOException ioe) {
			LOGGER.severe(makeMessage("IOException while reading string."));
			LOGGER.severe(makeMessage(ioe.getMessage()));
			throw new Exception(); // Java-style bad results handling
		}
	}

	// This method writes array of bytes to the socket
	private byte[] readBytesFromStream(DataInputStream stream) throws Exception {
		int byteMessageLength = readIntFromStream(stream);

		if (byteMessageLength == 0) {
			LOGGER.severe(makeMessage("Failed to read byte array."));
			throw new Exception();
		}

		try {
			byte[] byteMessage = new byte[byteMessageLength];
			stream.readFully(byteMessage);
			return byteMessage;
		} catch (EOFException eofe) {
			LOGGER.severe(makeMessage("Failed to read byte array, met EOF."));
			throw new Exception(); // Java-style bad results handling
		} catch (IOException ioe) {
			LOGGER.severe(makeMessage("IOException while reading byte array."));
			LOGGER.severe(makeMessage(ioe.getMessage()));
			throw new Exception(); // Java-style bad results handling
		}
	}

	// This method writes integer to the socket
	private void writeIntToStream(DataOutputStream stream, int intToWrite) throws Exception {
		byte[] bytes = ByteBuffer.allocate(4).putInt(intToWrite).array();
		try {
			stream.write(bytes);
		} catch (IOException ioe) {
			LOGGER.severe(makeMessage("IOException while writing integer."));
			LOGGER.severe(makeMessage(ioe.getMessage()));
			throw new Exception(); // Java-style bad results handling
		}
	}

	// This method will add working directory before file name.
	// For example this file name:
	//		cool_file.txt
	// will be transformed into
	//		./uploads/*client address*/cool_file.txt
	// Client MUST cut off everything but file name.
	// But it will be done on the server side just to be safe.
	private String makeFileName(String oldFileName) {
		int lastIndexOfSlash = oldFileName.lastIndexOf('/');

		// lastIndexOfSlash is in range from -1 to oldFileName.length - 2
		// There is no need to check if lastIndexOfSlash equals to string.length() - 1
		//	because if such file name is given, connection won't be started on a
		//	client side
		lastIndexOfSlash += 1;
		String clearFileName = oldFileName.substring(lastIndexOfSlash);

		return "./uploads/" + clientAddress + "/" + clearFileName;
	}

	// This method do next two steps:
	//	1) Checks if file exists, if it is, then return TRUE
	//	2) If it doesn't exist, then creates file and return FALSE
	private boolean createFile(String fileName) throws FileNotFoundException, IOException {
		receivedFile = new File(fileName);

		// Return TRUE if file already exists
		if (receivedFile.isFile()) {
			return true;
		}

		// First of all create directories if they do not exist
		File parentDirs = receivedFile.getParentFile();
		if (!parentDirs.exists()) {
			parentDirs.mkdirs();
		}

		// Create file
		receivedFile.createNewFile();

		// Return FALSE as notify that file has been created
		return false;

	}

	// This method writes array of bytes to the file stream.
	// File stream should be opened before.
	private void writeToFile(FileOutputStream fileOutputStream, byte[] packet) {
		try {
			fileOutputStream.write(packet);
		} catch (NullPointerException npe) {
			// Don't care
		} catch (IOException e) {
			LOGGER.warning(makeMessage("Failed to write data into file, file may be corrupted."));
			LOGGER.warning(makeMessage(e.getMessage()));
		}
	}

	@Override
	public void run() {

		try (Socket socket = passedSocket;
			 DataInputStream readStream = new DataInputStream(socket.getInputStream());
			 DataOutputStream writeStream = new DataOutputStream(socket.getOutputStream())
		) {
			LOGGER.info(makeMessage("Opened connection"));
			System.out.println("[" + clientAddress + "]: Opened connection.");

			// Read file name from socket
			String fileName;
			try {
				fileName = readStringFromStream(readStream);
			} catch (Exception e) {
				LOGGER.severe(makeMessage("Failed to read initial file data."));
				return;
			}
			LOGGER.info(makeMessage("Got file name from client."));

			// serverSideFileName now contains subdirectory
			String serverSideFileName = makeFileName(fileName);
			LOGGER.info(makeMessage("Transformed file name."));

			// File existence code is 1 or 2 because if something goes wrong,
			//	client will read 0 from socket. And 0 means that socket is closed
			//	on the server side.
			int fileExistenceCode = 2;
			try {
				boolean fileExists = createFile(serverSideFileName);
				if (fileExists) {
					LOGGER.info(makeMessage("File with name \"" + fileName + "\" already exists."));
					fileExistenceCode = 1;
				}
			} catch (FileNotFoundException fnfe) {
				LOGGER.severe(makeMessage("Failed to open file stream."));
				return;
			} catch (IOException ioe) {
				LOGGER.severe(makeMessage("Failed to create file."));
				return;
			}
			LOGGER.info(makeMessage("Opened \"" + serverSideFileName + "\" for writing."));

			// Send code of existence to the client
			try {
				writeIntToStream(writeStream, fileExistenceCode);
			} catch (Exception e) {
				LOGGER.severe(makeMessage("Failed to write code of file existence."));
				return;
			}
			LOGGER.info(makeMessage("Sent existence code to the client."));

			// Get decision of the client
			int overwriteFile;
			try {
				overwriteFile = readIntFromStream(readStream);
				if (overwriteFile == 0) {
					LOGGER.warning(makeMessage("Client is down."));
					return;
				}
			} catch (Exception e) {
				LOGGER.severe(makeMessage("Failed to read user's decision."));
				return;
			}
			LOGGER.info(makeMessage("Got client's decision."));

			// Start overwriting.
			// Case when file has not been previously stored on the server
			//	side, will be processed as rewriting decision (because these two cases
			//	have the same LOGGER logic).
			if (overwriteFile == 1) { // If received integer is 1, then rewrite file

				long startReceivingTime = Instant.now().getEpochSecond();

				// Start speed manager
				SpeedManager speedManager = new SpeedManager(startReceivingTime, PERIOD, clientAddress);
				ScheduledExecutorService speedManagerThreadExecutor = Executors.newScheduledThreadPool(1);

				speedManagerThreadExecutor.scheduleAtFixedRate(
						speedManager,
						PERIOD - 1,	// initial delay
						PERIOD,			// period
						TimeUnit.SECONDS
				);

				try (FileOutputStream fileOutputStream = new FileOutputStream(receivedFile)){
					while (true) {
						if (Thread.currentThread().isInterrupted()) {
							LOGGER.severe(makeMessage("Client handler is interrupted."));
							break;
						}

						byte[] packet = null;
						try {
							packet = readBytesFromStream(readStream);

							synchronized (speedManager.speedLock) {
								speedManager.bytesReadWithinPeriod += packet.length;
								speedManager.bytesReadTotal += packet.length;
							}

						} catch (Exception e) {
							LOGGER.severe(makeMessage("Failed to read incoming packet."));
							break;
						}

						writeToFile(fileOutputStream, packet);
					}
				}


				long endReceivingTime = Instant.now().getEpochSecond();
				long timeDelta = endReceivingTime - startReceivingTime;

				if (timeDelta == 0) {
					System.out.println("File was received too fast - speed can not be calculated.");
				}
				else {
					String endSpeedMessage = "[" + clientAddress + "]: Time: ";
					endSpeedMessage += timeDelta + "s.\n";
					endSpeedMessage += "\tAverage speed: ";
					endSpeedMessage += String.format(
							"%.2f",
							(double)speedManager.bytesReadTotal / (double)timeDelta
					) + "b/s\n";
					System.out.println(endSpeedMessage);
				}
				speedManagerThreadExecutor.shutdown();

				LOGGER.info(makeMessage("File \"" + fileName + "\" has been received."));
			}
			else if (overwriteFile == 2) { // If received integer is 2, then do nothing
				LOGGER.info(makeMessage("Client decided not to overwrite file."));
			}
			else { // If received integer is neither 1 nor 2, then LOGGER.log it and end the connection
				LOGGER.log(Level.SEVERE,
					makeMessage(
						"Got unexpected client decision."
							+ " Expected \"1\" or \"2\" but got "
							+ "\"" + overwriteFile + "\""
					)
				);
			}

		} catch (IOException ioe) {
			LOGGER.severe(makeMessage("Failed to open IO streams."));
			LOGGER.severe(ioe.getMessage());
		}
		finally {
			LOGGER.info(makeMessage("Closed connection"));
			System.out.println("[" + clientAddress + "]: Closed connection.");
		}
	}
}
