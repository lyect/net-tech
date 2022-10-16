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
import java.util.logging.Level;
import java.util.logging.Logger;

public class ClientRunnable implements Runnable {

    private final int bufferSize;

    private final String serverAddress;
    private final int serverPort;
    private final String fileToSendPath;
    private FileInputStream fileToSendInputStream;

    private Socket tcpSocket;
    private DataOutputStream writeStream;
    private DataInputStream readStream;
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

    public void close(){
        LOGGER.log(Level.INFO, "Closing connection with server...");

        int exceptionCount = 0;

        try {
            readStream.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Failed to close read stream properly.");
            LOGGER.log(Level.WARNING, ioe.getMessage());
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Closing not opened read stream.");
        }

        try {
            writeStream.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Failed to close write stream properly.");
            LOGGER.log(Level.WARNING, ioe.getMessage());
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Closing not opened write stream.");
        }

        try {
            tcpSocket.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Failed to close socket properly.");
            LOGGER.log(Level.WARNING, ioe.getMessage());
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Closing not opened socket.");
        }

        try {
            fileToSendInputStream.close();
        } catch (IOException ioe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Failed to close file stream properly.");
            LOGGER.log(Level.WARNING, ioe.getMessage());
        } catch (NullPointerException npe) {
            exceptionCount += 1;
            LOGGER.log(Level.WARNING, "Closing not opened file stream.");
        }

        if (exceptionCount == 0) {
            LOGGER.log(Level.INFO, "Successfully closed connection.");
        }
        else {
            LOGGER.log(Level.WARNING, "Connection closed with errors.");
        }

        System.out.println("Connection is closed.");

    }

    private String cleanupFileName(String oldFileName) {
        int lastIndexOfSlash = oldFileName.lastIndexOf('/');

        // lastIndexOfSlash is in range from -1 to oldFileName.length - 2
        // If lastIndexOfSlash is oldFileName.length - 1 then this method
        //	won't even start.
        lastIndexOfSlash += 1;

        return oldFileName.substring(lastIndexOfSlash);
    }

    private void writeInt(int intToWrite) throws Exception {
        byte[] bytes = ByteBuffer.allocate(4).putInt(intToWrite).array();
        try {
            writeStream.write(bytes);
        } catch (IOException ioe) {
            LOGGER.log(Level.SEVERE, "IOException while writing integer.");
            LOGGER.log(Level.SEVERE, ioe.getMessage());
            throw new Exception(); // Java-style bad results handling
        }
    }

    private void writeBytes(int bytesLength, byte[] bytesToWrite) throws Exception {
        writeInt(bytesLength);

        try {
            writeStream.write(bytesToWrite);
        } catch (IOException ioe) {
            LOGGER.log(Level.SEVERE, "IOException while writing bytes.");
            LOGGER.log(Level.SEVERE, ioe.getMessage());
            throw new Exception(); // Java-style bad results handling
        }
    }

    private void writeString(String stringToWrite) throws Exception {
        byte[] byteString = stringToWrite.getBytes(StandardCharsets.UTF_8);
        writeBytes(byteString.length, byteString);
    }

    private int readInt() throws Exception {
        try {
            return readStream.readInt();
        } catch (EOFException eofe) {
            LOGGER.log(Level.SEVERE, "Failed to read integer, met EOF.");
            throw new Exception(); // Java-style bad results handling
        } catch (IOException ioe) {
            LOGGER.log(Level.SEVERE, "IOException while reading integer.");
            LOGGER.log(Level.SEVERE, ioe.getMessage());
            throw new Exception(); // Java-style bad results handling
        }
    }

    @Override
    public void run() {

        // Connect to the server and open io streams
        try {
            tcpSocket = new Socket(serverAddress, serverPort, InetAddress.getLocalHost(), localPort);
            writeStream = new DataOutputStream(tcpSocket.getOutputStream());
            readStream = new DataInputStream(tcpSocket.getInputStream());
        } catch (IOException uhe) {
            LOGGER.log(Level.SEVERE, "Failed to connect to server.");
            System.out.println("Failed to connect to server.");
            close();
            return;
        }

        LOGGER.log(Level.INFO, "Connected to server.");
        System.out.println("Connected to server.");

        // Open file which will be sent
        File fileToSend = new File(fileToSendPath);

        // Check if file exists
        if (!fileToSend.isFile()) {
            LOGGER.log(Level.SEVERE, "Passed path does not lead to a file.");
            close();
            return;
        }

        LOGGER.log(Level.INFO, "Opened file with path \"" + fileToSendPath + "\".");

        // Open file stream to read from the file
        try {
            fileToSendInputStream = new FileInputStream(fileToSend);
        } catch (FileNotFoundException e) {
            LOGGER.log(Level.SEVERE, "Failed to open a file stream.");
            close();
            return;
        }

        LOGGER.log(Level.INFO, "Opened file stream.");

        // Send file name to the server
        String cleanFileName = cleanupFileName(fileToSendPath);
        try {
            writeString(cleanFileName);
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Failed to send file name.");
            close();
            return;
        }

        LOGGER.log(Level.INFO, "Sent file name.");

        // Server responds with 1 or 2.
        // 1 means that file with same name is stored on server.
        // 2 means that server has no file with this name.
        int serverResponse;
        try {
            serverResponse = readInt();
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Failed to read server response.");
            close();
            return;
        }

        if (serverResponse == 0) {
            LOGGER.log(Level.SEVERE, "Server is down.");
            close();
            return;
        }

        LOGGER.log(Level.INFO, "Got server's response.");

        int userAnswerCode = 1;

        if (serverResponse == 1) {
            System.out.println("There is a file with the same name on the server.");
            System.out.println("Do you want to overwrite it? [Y/N]");

            BufferedReader terminalReader = new BufferedReader(new InputStreamReader(System.in));
            while (true) {
                try {
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
                } catch (IOException ioe) {
                    LOGGER.log(Level.WARNING, "Failed to read line from terminal.");
                    LOGGER.log(Level.WARNING, ioe.getMessage());
                }
            }

            try {
                terminalReader.close();
            } catch (IOException ioe) {
                LOGGER.log(Level.WARNING, "Failed to close terminal reader properly.");
                LOGGER.log(Level.WARNING, ioe.getMessage());
            }
        }

        try {
            writeInt(userAnswerCode);
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Failed to send decision");
            close();
            return;
        }

        LOGGER.log(Level.INFO, "Sent client's decision.");

        if (userAnswerCode == 1) {
            byte[] buffer = new byte[bufferSize];
            int bytesRead;

            while (true) {
                try {
                    bytesRead = fileToSendInputStream.read(buffer);
                    if (bytesRead <= 0) {
                        break;
                    }
                    try {
                        writeBytes(bytesRead, buffer);
                    } catch (Exception e) {
                        LOGGER.log(Level.SEVERE, "Failed to send data.");
                        close();
                        return;
                    }
                } catch (IOException e1) {
                    LOGGER.log(Level.SEVERE, "Failed to read data from file.");
                    close();
                    return;
                }
            }

            LOGGER.log(Level.INFO, "Successfully sent \"" + cleanFileName + "\".");
            System.out.println("Successfully sent \"" + cleanFileName + "\".");
        }
        else {
            LOGGER.log(Level.INFO, "Client decided not to rewrite file.");
        }

        close();
    }
}
