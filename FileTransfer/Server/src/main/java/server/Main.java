package server;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

public class Main {

    private static int PORT;
    private static Logger LOGGER;

    private static void setUpLoggerConfiguration() throws SecurityException, IOException {
        InputStream ins = Main.class.getResourceAsStream("/logger.properties");
        LogManager.getLogManager().readConfiguration(ins);
    }

    public static void main(String[] args) throws SecurityException, IOException, Exception{

        // Load configuration of the logger
        setUpLoggerConfiguration();

        // Check if appropriate number of arguments is given
        if (args.length < 1) {
            throw new Exception("Not enough arguments! Please, specify server's listening port.");
        }
        if (args.length > 1) {
            throw new Exception("Too many arguments! There must be only server's listening port.");
        }

        // Get port on which server will listen
        try {
            PORT = Integer.parseInt(args[0]);
        } catch (Exception e) {
            System.out.println("Incorrect port!");
            return;
        }

        // Get logger for class "Main"
        LOGGER = Logger.getLogger(Main.class.getName());

        // Start thread with logic of the server
        Thread serverThread = new Thread(new ServerRunnable(PORT));
        serverThread.start();

        // Start busy-waiting of STOP command
        System.out.println("Enter \"STOP\" when you want to stop");
        BufferedReader terminalReader = new BufferedReader(new InputStreamReader(System.in));
        while (true) {
            String inputCommand = terminalReader.readLine().strip();
            if (inputCommand.equals("STOP")) {
                break;
            }
        }

        LOGGER.log(Level.INFO, "Closing terminal reader...");
        terminalReader.close();
        LOGGER.log(Level.INFO, "Terminal reader is closed.");

        // Interrupt server
        LOGGER.log(Level.INFO, "Interrupting server...");
        serverThread.interrupt();
        serverThread.join();
        LOGGER.log(Level.INFO, "Server is interrupted.");
    }

}
