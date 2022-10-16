package copydetector;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.MulticastSocket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.time.LocalTime;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.BlockingQueue;

public class Listener implements Runnable {

    private MulticastSocket multicastSocket;

    private BlockingQueue<DatagramPacket> messageQueue;

    private int resendingNonce;
    private int key;

    private int outdatingSeconds;

    public Listener(
            MulticastSocket _multicastSocket,
            BlockingQueue<DatagramPacket> _messageQueue,
            int _resendingNonce,
            int _key,
            int _outdatingSeconds
    ) {
        multicastSocket = _multicastSocket;

        messageQueue = _messageQueue;

        resendingNonce = _resendingNonce;
        key = _key;

        outdatingSeconds = _outdatingSeconds;
    }

    private void printHostsList(HashMap<String, HostData> knownHosts) {
        int numberOfHostsOnline = 0;
        System.out.println("List of online hosts has changed:");
        for (String host : knownHosts.keySet()) {
            numberOfHostsOnline += 1;
            System.out.println(host);
        }

        if (numberOfHostsOnline == 0) {
            System.out.println("*Nobody is online*");
        }
    }

    @Override
    public void run() {
        HashMap<String, HostData> knownHosts = new HashMap<>();

        Timer catchOutdatedHostsTimer = new Timer();

        catchOutdatedHostsTimer.schedule(new TimerTask() {
            public void run() {
                synchronized (knownHosts) {
                    int outdatedHostsCount = 0;
                    LocalTime currentTime = LocalTime.now();

                    Iterator<HashMap.Entry<String, HostData>> knownHostsIterator = knownHosts.entrySet().iterator();

                    while (knownHostsIterator.hasNext()) {
                        HashMap.Entry<String, HostData> entry = knownHostsIterator.next();

                        String host = entry.getKey();
                        HostData hostData = entry.getValue();

                        if (hostData.isOutdated(currentTime, outdatingSeconds)) {
                            System.out.println(host + " outdated.");
                            System.out.print("\tLast packet from " + host + " was at ");
                            System.out.println(hostData.getTime());
                            System.out.print("\tBut now is ");
                            System.out.println(currentTime);
                            knownHostsIterator.remove();
                            outdatedHostsCount += 1;
                        }
                    }

                    if (outdatedHostsCount != 0) {
                        printHostsList(knownHosts);
                    }
                }
            }
        }, 0, outdatingSeconds * 1000);

        while (true) {
            if (Thread.currentThread().isInterrupted()) {
                catchOutdatedHostsTimer.cancel();
                break;
            }

            DatagramPacket receivedDatagram = new DatagramPacket(new byte[400], 400);

            try {
                multicastSocket.receive(receivedDatagram);
            } catch (IOException e) {
                continue;
            }

            String sourceAddress = receivedDatagram.getAddress().getHostAddress();
            ByteBuffer byteContent = ByteBuffer.wrap(receivedDatagram.getData());

            int receivedHash = byteContent.getInt();
            int receivedNonce = byteContent.getInt();

            synchronized (knownHosts) {
                System.out.print("New packet from " + sourceAddress + " at ");
                System.out.println(LocalTime.now());
                System.out.print("\tReceivedNonce: ");
                System.out.println(receivedNonce);
                System.out.print("\tStoredNonce: ");
                HostData hd = knownHosts.get(sourceAddress);
                if (hd == null) {
                    System.out.println("null");
                }
                else {
                    System.out.println(hd.getNonce());
                }
            }
            if (receivedHash == GreatHasher.hashFunction2(receivedNonce, key)) {
                if (!knownHosts.containsKey(sourceAddress)) {
                    synchronized (knownHosts) {
                        System.out.println(sourceAddress + " online.");
                        knownHosts.put(sourceAddress, new HostData(receivedNonce));
                        printHostsList(knownHosts);
                    }

                    try {
                        DatagramPacket repeatPacket = PacketMaker.makePacket(resendingNonce);
                        messageQueue.put(repeatPacket);
                    }
                    catch (InterruptedException e) {
                        catchOutdatedHostsTimer.cancel();
                        break;
                    }
                    catch (UnknownHostException e) {
                        // Don't care
                    }
                }
                else {

                    int storedNonce = knownHosts.get(sourceAddress).getNonce();

                    if (receivedNonce == storedNonce + 1) {
                        synchronized (knownHosts) {
                            System.out.println(sourceAddress + " offline.");
                            knownHosts.remove(sourceAddress);
                            printHostsList(knownHosts);
                        }
                    }

                    if (receivedNonce == storedNonce) {
                        synchronized (knownHosts) {
                            knownHosts.replace(sourceAddress, new HostData(receivedNonce));
                        }
                    }
                }
            }
        }
    }
}
