package copydetector;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.MulticastSocket;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.BlockingQueue;

public class Sender implements Runnable {

    private BlockingQueue<DatagramPacket> messageQueue;

    private MulticastSocket multicastSocket;

    private int nonce;

    private int resendingSeconds;

    public Sender(
            MulticastSocket _multicastSocket,
            BlockingQueue<DatagramPacket> _messageQueue,
            int _nonce,
            int _resendingSeconds
    ) {
        messageQueue = _messageQueue;
        multicastSocket = _multicastSocket;
        nonce = _nonce;
        resendingSeconds = _resendingSeconds;
    }

    @Override
    public void run() {

        Timer sendAlivenessPacketTimer = new Timer();

        sendAlivenessPacketTimer.schedule(new TimerTask() {
            public void run() {
                try {
                    multicastSocket.send(PacketMaker.makePacket(nonce));
                } catch (IOException e) {
                    // Don't care
                }
            }
        }, 0, resendingSeconds * 1000);

        while (true) {

            if (Thread.currentThread().isInterrupted()) {
                sendAlivenessPacketTimer.cancel();
                break;
            }

            try {
                DatagramPacket message = messageQueue.take();
                multicastSocket.send(message);
            } catch (InterruptedException e) {
                break;
            } catch (IOException e) {
                // Don't care
            }
        }
    }
}
