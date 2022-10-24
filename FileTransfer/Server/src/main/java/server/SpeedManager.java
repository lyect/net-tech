package server;

import java.time.Instant;

public class SpeedManager implements Runnable {

    private final long startReceivingTime;
    private final int period;
    // Stores number of all bytes received from client within period.
    // After calculating time in period will be set to 0. This action will be
    //	done by SpeedManager
    public int bytesReadWithinPeriod;
    // Stores number of all bytes received from client
    public int bytesReadTotal;
    // Mutex for changing bytesSentWithinPeriod and bytesSentTotal
    public final Object speedLock;
    private final String clientAddress;

    public SpeedManager(
            long _startReceivingTime,
            int _period,
            String _clientAddress
    ) {
        startReceivingTime = _startReceivingTime;
        period = _period;
        bytesReadWithinPeriod = 0;
        bytesReadTotal = 0;
        speedLock = new Object();
        clientAddress = _clientAddress;
    }

    @Override
    public void run() {
        String message = "[" + clientAddress + "]: ";

        long currentTime = Instant.now().getEpochSecond();
        long timeDelta = currentTime - startReceivingTime;

        message += "Time: " + timeDelta + "s.\n";

        synchronized (speedLock) {
            message += "\tAverage speed: ";
            message += String.format("%.2f", (float)bytesReadTotal / timeDelta);
            message += "b/s\n";

            message += "\tSpeed in last " + period + " seconds: ";
            message += String.format("%.2f", (float)bytesReadWithinPeriod / timeDelta);
            message += "b/s\n";

            bytesReadWithinPeriod = 0;
        }

        System.out.println(message);

    }
}
