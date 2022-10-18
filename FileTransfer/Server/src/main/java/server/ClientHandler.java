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

    private final Socket socket;
    private final String clientAddress;

    private DataInputStream readStream;
    private DataOutputStream writeStream;

    private boolean streamsOpened;

    private File receivedFile;
    private FileOutputStream fileOutputStream;
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

        socket = _socket;

        // Get client address without port and forward slash
        clientAddress = (((InetSocketAddress)socket.getRemoteSocketAddress()).getAddress()).toString().replace("/","");

        // If streams are not opened, then this variable will be set to FALSE.
        // It means that client handler can not be started properly
        //	and start-up must immediately end with an error thrown
        streamsOpened = false;

        try {
            readStream = new DataInputStream(socket.getInputStream());
            try {
                writeStream = new DataOutputStream(socket.getOutputStream());
                streamsOpened = true;
            } catch (IOException ioe) {
                LOGGER.severe(makeMessage("Failed to open write stream."));
                LOGGER.severe(ioe.getMessage());
            }
        } catch (IOException ioe) {
            LOGGER.severe(makeMessage("Failed to open read stream."));
            LOGGER.severe(makeMessage(ioe.getMessage()));
        }

        if (streamsOpened) {
            LOGGER.info(makeMessage("Opened connection"));
            System.out.println("[" + clientAddress + "]: Opened connection.");
        }
    }

    private void close() {
        LOGGER.info(makeMessage("Closing connection..."));

        int exceptionCount = 0;

        try {
            fileOutputStream.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Failed to close file stream properly."));
            LOGGER.warning(makeMessage(ioe.getMessage()));
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Closing not opened file stream."));
        }

        try {
            readStream.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Failed to close read stream properly."));
            LOGGER.warning(makeMessage(ioe.getMessage()));
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Closing not opened read stream."));
        }

        try {
            writeStream.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Failed to close write stream properly."));
            LOGGER.warning(makeMessage(ioe.getMessage()));
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Closing not opened write stream."));
        }

        try {
            socket.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Failed to close handler's socket properly."));
            LOGGER.warning(makeMessage(ioe.getMessage()));
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.warning(makeMessage("Closing not opened socket."));
        }

        if (exceptionCount == 0) {
            LOGGER.info(makeMessage("Successfully closed connection."));
        }
        else {
            LOGGER.warning(makeMessage("Connection closed with errors."));
        }

        System.out.println("[" + clientAddress + "]: Closed connection.");
    }

    // This method reads integer from the socket
    private int readInt() throws Exception {
        try {
            return readStream.readInt();
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
    private String readString() throws Exception {
        int byteMessageLength = readInt();

        if (byteMessageLength == 0) {
            LOGGER.severe(makeMessage("Failed to read string."));
            throw new Exception();
        }

        try {
            byte[] byteMessage = new byte[byteMessageLength];
            readStream.readFully(byteMessage);
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
    private byte[] readBytes() throws Exception {
        int byteMessageLength = readInt();

        if (byteMessageLength == 0) {
            LOGGER.severe(makeMessage("Failed to read byte array."));
            throw new Exception();
        }

        try {
            byte[] byteMessage = new byte[byteMessageLength];
            readStream.readFully(byteMessage);
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
    private void writeInt(int intToWrite) throws Exception {
        byte[] bytes = ByteBuffer.allocate(4).putInt(intToWrite).array();
        try {
            writeStream.write(bytes);
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
    //	1) Checks if file exists, if it is, then will open fileStream and return TRUE
    //	2) If it doesn't exist,  then creates file, will open fileStream and return FALSE
    private boolean createFile(String fileName) throws FileNotFoundException, IOException {
        receivedFile = new File(fileName);

        // Return TRUE if file already exists
        if (receivedFile.isFile()) {
            fileOutputStream = new FileOutputStream(receivedFile);
            return true;
        }

        // First of all create directories if they do not exist
        File parentDirs = receivedFile.getParentFile();
        if (!parentDirs.exists()) {
            parentDirs.mkdirs();
        }

        // Create file
        receivedFile.createNewFile();

        // Open outputStream
        fileOutputStream = new FileOutputStream(receivedFile);

        // Return FALSE as notify that file has been created
        return false;

    }

    // This method writes array of bytes to the file stream.
    // File stream should be opened before.
    private void writeToFile(byte[] packet) {
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

        // If one of the streams (or two of them) is not opened,
        //	then client handler is not able to start
        if (!streamsOpened) {
            LOGGER.severe(makeMessage("Streams are not opened, closing client handler."));
            close();
            return;
        }

        // Read file name
        String fileName;
        try {
            fileName = readString();
        } catch (Exception e) {
            LOGGER.severe(makeMessage("Failed to read initial file data."));
            close();
            return;
        }
        LOGGER.info(makeMessage("Got file name from client."));

        if (Thread.currentThread().isInterrupted()) {
            LOGGER.info(makeMessage("Client handler is interrupted."));
            close();
            return;
        }

        // serverSideFileName now contains subdirectory
        String serverSideFileName = makeFileName(fileName);

        // TRUE if file exists FALSE otherwise
        boolean fileExists;
        try {
            fileExists = createFile(serverSideFileName);
        } catch (FileNotFoundException fnfe) {
            LOGGER.severe(makeMessage("Failed to open file stream."));
            close();
            return;
        } catch (IOException ioe) {
            LOGGER.severe(makeMessage("Failed to create file."));
            close();
            return;
        }
        LOGGER.info(makeMessage("Transformed file name."));

        // File existence code is 1 or 2 because if something goes wrong,
        //	client will read 0 from socket. And 0 means that socket is closed
        //	on the server side.
        int fileExistenceCode = 2;
        if (fileExists) {
            LOGGER.info(makeMessage("File with name \"" + fileName + "\" already exists."));
            fileExistenceCode = 1;
        }
        LOGGER.info(makeMessage("Opened \"" + serverSideFileName + "\" for writing."));

        // Send code of existence to the client
        try {
            writeInt(fileExistenceCode);
        } catch (Exception e) {
            LOGGER.severe(makeMessage("Failed to write code of file existence."));
            close();
            return;
        }
        LOGGER.info(makeMessage("Sent existence code to the client."));

        if (Thread.currentThread().isInterrupted()) {
            LOGGER.info(makeMessage("Client handler is interrupted."));
            close();
            return;
        }

        // Get decision of the client
        int overwriteFile;
        try {
            overwriteFile = readInt();
        } catch (Exception e) {
            LOGGER.severe(makeMessage("Failed to read user's decision."));
            close();
            return;
        }

        if (overwriteFile == 0) {
            LOGGER.warning(makeMessage("Client is down."));
            close();
            return;
        }
        LOGGER.info(makeMessage("Got client's decision."));

        if (Thread.currentThread().isInterrupted()) {
            LOGGER.severe(makeMessage("Client handler is interrupted."));
            close();
            return;
        }

        // Start overwriting.
        // Case when file has not been previously stored on the server
        //	side, will be processed as rewriting decision (because these two cases
        //	have the same LOGGER.logic).
        if (overwriteFile == 1) { // If received integer is 1, then rewrite file

            long startReceivingTime = Instant.now().getEpochSecond();

            // Start speed manager
            SpeedManager speedManager = new SpeedManager(startReceivingTime, PERIOD, clientAddress);
            ScheduledExecutorService speedManagerThreadExecutor = Executors.newScheduledThreadPool(1);

            speedManagerThreadExecutor.scheduleAtFixedRate(
                    speedManager,
                    PERIOD - 1, // initial delay
                    PERIOD,     // period
                    TimeUnit.SECONDS
            );

            while (true) {
                if (Thread.currentThread().isInterrupted()) {
                    LOGGER.severe(makeMessage("Client handler is interrupted."));
                    break;
                }

                byte[] packet = null;
                try {
                    packet = readBytes();

                    synchronized (speedManager.speedLock) {
                        speedManager.bytesReadWithinPeriod += packet.length;
                        speedManager.bytesReadTotal += packet.length;
                    }

                } catch (Exception e) {
                    LOGGER.severe(makeMessage("Failed to read incoming packet."));
                    break;
                }

                writeToFile(packet);
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
                endSpeedMessage += String.format("%.2f", (double)speedManager.bytesReadTotal / (double)timeDelta) + "b/s\n";
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

        close();
    }
}
