package client;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.logging.Logger;

public class ClientRunnable implements Runnable {

	private final int bufferSize;

	private final String serverAddress;
	private final int serverPort;
	private final String fileToSendPath;

	private final int localPort;

	private static Logger LOGGER;

	public ClientRunnable(
			int _bufferSize,
			String _serverAddress,
			int _serverPort,
			int _localPort,
			String _fileToSendPath
	) {
		LOGGER = Logger.getLogger(ClientRunnable.class.getName());
		bufferSize = _bufferSize;
		serverAddress = _serverAddress;
		serverPort = _serverPort;
		localPort = _localPort;
		fileToSendPath = _fileToSendPath;
	}

	private String cleanupFileName(String oldFileName) {
		int lastIndexOfSlash = oldFileName.lastIndexOf('/');

		// lastIndexOfSlash is in range from -1 to oldFileName.length - 2
		// If lastIndexOfSlash is oldFileName.length - 1 then this method
		//	won't even start.
		lastIndexOfSlash += 1;

		return oldFileName.substring(lastIndexOfSlash);
	}

	private void writeIntToStream(DataOutputStream stream, int intToWrite) throws Exception {
		byte[] bytes = ByteBuffer.allocate(4).putInt(intToWrite).array();
		try {
			stream.write(bytes);
		} catch (IOException ioe) {
			LOGGER.severe("IOException while writing integer.");
			LOGGER.severe(ioe.getMessage());
			throw new Exception(); // Java-style bad results handling
		}
	}

	private void writeBytesToStream(DataOutputStream stream, int bytesLength, byte[] bytesToWrite) throws Exception {
		writeIntToStream(stream, bytesLength);

		try {
			stream.write(bytesToWrite);
		} catch (IOException ioe) {
			LOGGER.severe("IOException while writing bytes.");
			LOGGER.severe(ioe.getMessage());
			throw new Exception(); // Java-style bad results handling
		}
	}

	private void writeStringToStream(DataOutputStream stream, String stringToWrite) throws Exception {
		byte[] byteString = stringToWrite.getBytes(StandardCharsets.UTF_8);
		writeBytesToStream(stream, byteString.length, byteString);
	}

	private int readIntFromStream(DataInputStream stream) throws Exception {
		try {
			return stream.readInt();
		} catch (EOFException eofe) {
			LOGGER.severe("Failed to read integer, met EOF.");
			throw new Exception(); // Java-style bad results handling
		} catch (IOException ioe) {
			LOGGER.severe("IOException while reading integer.");
			LOGGER.severe(ioe.getMessage());
			throw new Exception(); // Java-style bad results handling
		}
	}

	@Override
	public void run() {

		// Connect to the server and open io streams
		try (Socket tcpSocket = new Socket(serverAddress, serverPort, InetAddress.getLocalHost(), localPort);
			 DataOutputStream writeStream = new DataOutputStream(tcpSocket.getOutputStream());
			 DataInputStream readStream = new DataInputStream(tcpSocket.getInputStream())
		) {
			LOGGER.info("Connected to server.");
			System.out.println("Connected to server.");

			// Open file which will be sent
			File fileToSend = new File(fileToSendPath);

			// Check if file exists
			if (!fileToSend.isFile()) {
				LOGGER.severe("Passed path does not lead to a file.");
				System.out.println("This file does not exist or it is a directory.");
				return;
			}
			LOGGER.info("Opened file with path \"" + fileToSendPath + "\".");

			// Send file name to the server
			String cleanFileName = cleanupFileName(fileToSendPath);
			try {
				writeStringToStream(writeStream, cleanFileName);
			} catch (Exception e) {
				LOGGER.severe("Failed to send file name.");
				return;
			}
			LOGGER.info("Sent file name.");

			// After receiving file name, server responds with 1 or 2.
			// 1 means that file with same name is stored on server.
			// 2 means that server has no file with this name.
			int serverResponse;
			try {
				serverResponse = readIntFromStream(readStream);

				// If server is down, 0 will be read from input stream
				if (serverResponse == 0) {
					LOGGER.severe("Server is down.");
					return;
				}
			} catch (Exception e) {
				LOGGER.severe("Failed to read server response.");
				return;
			}
			LOGGER.info("Got server's response.");

			// userAnswerCode = 1 means that user want to send file to server.
			//	If server has file with the same name on its side,
			//	that file will be overwritten.
			// userAnswerCode = 2 means that user do not want to overwrite old file
			int userAnswerCode = 1;

			// Case if server has file with the same name
			if (serverResponse == 1) {
				System.out.println("There is a file with the same name on the server.");
				System.out.println("Do you want to overwrite it? [Y/N]");

				try (InputStreamReader systemInReader = new InputStreamReader(System.in);
					 BufferedReader terminalReader = new BufferedReader(systemInReader)
				) {
					while (true) {
						String userAnswerString = terminalReader.readLine();

						if (userAnswerString.equals("Y")) {
							break;
						}
						else if (userAnswerString.equals("N")) {
							userAnswerCode = 2;
							break;
						}
						else {
							System.out.println("Do you want to overwrite it? [Y/N]");
						}
					}
				} catch (IOException ioe) {
					LOGGER.warning("Catch exception while reading from terminal.");
					LOGGER.warning(ioe.getMessage());
					return;
				}
			}

			// Send user decision to the server
			try {
				writeIntToStream(writeStream, userAnswerCode);
			} catch (Exception e) {
				LOGGER.severe("Failed to send decision");
				return;
			}
			LOGGER.info("Sent client's decision.");

			// If user want to overwrite old file (or send new one)
			if (userAnswerCode == 1) {
				byte[] buffer = new byte[bufferSize];
				int bytesRead;

				try (FileInputStream fileToSendInputStream = new FileInputStream(fileToSend)) {
					LOGGER.info("Opened file stream.");

					while (true) {
						// Read new chunk of file
						bytesRead = fileToSendInputStream.read(buffer);
						if (bytesRead <= 0) {
							break;
						}
						// Send chunk to server
						try {
							writeBytesToStream(writeStream, bytesRead, buffer);
						} catch (Exception e) {
							LOGGER.severe("Failed to send data.");
							return;
						}
					}
				} catch (FileNotFoundException e){
					LOGGER.severe("File not found.");
					return;
				} catch (IOException e1) {
					LOGGER.severe("Failed to read data from file.");
					return;
				}

				LOGGER.info("Successfully sent \"" + cleanFileName + "\".");
				System.out.println("Successfully sent \"" + cleanFileName + "\".");
			}
			else {
				LOGGER.info("Client decided not to rewrite file.");
			}
		} catch (IOException ioe) {
			LOGGER.severe("Catch exception while opening socket.");
			LOGGER.severe(ioe.getMessage());
		}
		finally {
			LOGGER.info("Disconnected from the server.");
			System.out.println("Disconnected from the server.");
		}
	}
}