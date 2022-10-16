package copydetector;

import java.time.LocalTime;
import java.time.temporal.ChronoUnit;

public class HostData {
    private int nonce;
    private LocalTime lastPacketTime;

    public HostData(int _nonce) {
        nonce = _nonce;
        lastPacketTime = LocalTime.now();
    }

    public int getNonce() {
        return nonce;
    }

    public LocalTime getTime() {
        return lastPacketTime;
    }

    public boolean isOutdated(LocalTime currentTime, int outdatingSeconds) {
        long secondsSinceLastPacket = ChronoUnit.SECONDS.between(lastPacketTime, currentTime);

        if (secondsSinceLastPacket >= outdatingSeconds) {
            return true;
        }
        return false;
    }
}
