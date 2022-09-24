package copydetector;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.MulticastSocket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.concurrent.BlockingQueue;

public class Listener implements Runnable {
	
	private MulticastSocket multicastSocket;
	
	private BlockingQueue<DatagramPacket> messageQueue;
	
	private int resendingNonce;
	private int key;
	
	public Listener(MulticastSocket _multicastSocket, BlockingQueue<DatagramPacket> _messageQueue, int _resendingNonce, int _key) {
		multicastSocket = _multicastSocket;
		
		messageQueue = _messageQueue;
		
		resendingNonce = _resendingNonce;
		key = _key;
	}
	
	private void printHostsList(HashMap<String, Integer> knownHosts) {
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
		HashMap<String, Integer> knownHosts = new HashMap<>();

		while (true) {
			if (Thread.currentThread().isInterrupted()) {
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

			if (receivedHash == GreatHasher.hashFunction2(receivedNonce, key)) {
				if (!knownHosts.containsKey(sourceAddress)) {
					System.out.println(sourceAddress + " online.");
					knownHosts.put(sourceAddress, receivedNonce);
					printHostsList(knownHosts);
					
					try {
						DatagramPacket repeatPacket = PacketMaker.makePacket(resendingNonce);
						messageQueue.put(repeatPacket);
					}
					catch (InterruptedException e) {
						break;
					}
					catch (UnknownHostException e) {
						// Don't care
					}
				}
				else {

					int storedNonce = knownHosts.get(sourceAddress);

					if (receivedNonce == storedNonce + 1) {
						System.out.println(sourceAddress + " offline.");
						knownHosts.remove(sourceAddress);
						printHostsList(knownHosts);
					}
				}
			}
		}
		
	}
	
}
