package client;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

import org.apache.commons.validator.routines.InetAddressValidator;

public class Main {

	private final static int LOCAL_PORT = 8000;
	private final static int BUFFER_SIZE = 1024 * 1024;
	private static Logger LOGGER;

	private static void setUpLoggerConfiguration() throws SecurityException, IOException {
		InputStream ins = Main.class.getResourceAsStream("/logger.properties");
		LogManager.getLogManager().readConfiguration(ins);
	}

	public static void main(String[] args) throws Exception {

		setUpLoggerConfiguration();

		if (args.length < 2) {
			throw new Exception("Not enough arguments. Usage:\n\tjava -jar client.jar [server ip] [port]");
		}

		if (args.length > 2) {
			throw new Exception("Too much arguments. Usage:\n\tjava -jar client.jar [server ip] [port]");
		}

		// Parse command-line arguments
		String serverAddress = args[0];

		InetAddressValidator validator = InetAddressValidator.getInstance();
		if (!validator.isValid(serverAddress)) {
			System.out.println("Incorrect server address!");
			return;
		}

		int serverPort;
		try {
			serverPort = Integer.parseInt(args[1]);
		} catch (Exception e) {
			System.out.println("Incorrect server port!");
			return;
		}

		// Get logger for class "Main"
		LOGGER = Logger.getLogger(Main.class.getName());

		// Enter a path to a file which will be sent
		System.out.println("Enter a name of the file you want to send to server:");

		BufferedReader terminalReader = new BufferedReader(new InputStreamReader(System.in));
		String filePathString = terminalReader.readLine();

		// Create thread with client logic
		Thread clientThread = new Thread(new ClientRunnable(
				BUFFER_SIZE,
				serverAddress,
				serverPort,
				LOCAL_PORT,
				filePathString
		));
		clientThread.start();
		clientThread.join();

		LOGGER.log(Level.INFO, "Closing terminal reader...");
		terminalReader.close();
		LOGGER.log(Level.INFO, "Terminal reader is closed.");
	}
}
